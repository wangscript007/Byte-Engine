#include "GameApplication.h"

#include "ByteEngine/Application/InputManager.h"
#include "ByteEngine/Debug/FunctionTimer.h"
#include "ByteEngine/Game/CameraSystem.h"
#include "ByteEngine/Game/GameInstance.h"
#include "ByteEngine/Render/FrameManager.h"
#include "ByteEngine/Render/MaterialSystem.h"
#include "ByteEngine/Render/RenderOrchestrator.h"
#include "ByteEngine/Render/StaticMeshRenderGroup.h"

#include "ByteEngine/Render/RenderSystem.h"
#include "ByteEngine/Render/TextSystem.h"
#include "ByteEngine/Render/TextureSystem.h"
#include "ByteEngine/Resources/MaterialResourceManager.h"
#include "ByteEngine/Resources/PipelineCacheResourceManager.h"
#include "ByteEngine/Resources/StaticMeshResourceManager.h"
#include "ByteEngine/Resources/TextureResourceManager.h"

#include "ByteEngine/Resources/AudioResourceManager.h"

#pragma comment(lib, "XInput.lib")

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC ((unsigned short) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE ((unsigned short) 0x02)
#endif

class RenderOrchestrator;

void GameApplication::Initialize()
{
	Application::Initialize();

	GTSL::Window::WindowCreateInfo create_window_info;
	create_window_info.Application = &systemApplication;
	create_window_info.Name = GTSL::StaticString<1024>(GetApplicationName());
	create_window_info.Extent = { 1280, 720 };
	::new(&window) GTSL::Window(create_window_info);

	window.SetOnWindowResizeDelegate(GTSL::Delegate<void(const GTSL::Extent2D&)>::Create<GameApplication, &GameApplication::onWindowResize>(this));

	auto window_close = []()
	{
		Get()->PromptClose();
		Get()->Close(CloseMode::OK, GTSL::Ranger<const UTF8>());
	};
	window.SetOnCloseDelegate(GTSL::Delegate<void()>::Create(window_close));

	auto window_move = [](uint16 x, uint16 y)
	{

	};
	window.SetOnWindowMoveDelegate(GTSL::Delegate<void(uint16, uint16)>::Create(window_move));
	
	SetupInputSources();
	
	CreateResourceManager<StaticMeshResourceManager>();
	CreateResourceManager<TextureResourceManager>();
	CreateResourceManager<MaterialResourceManager>();
	CreateResourceManager<AudioResourceManager>();
	CreateResourceManager<PipelineCacheResourceManager>();
	CreateResourceManager<FontResourceManager>();
}

void GameApplication::PostInitialize()
{	
	gameInstance->AddGoal("FrameStart");
	gameInstance->AddGoal("GameplayStart");
	gameInstance->AddGoal("GameplayEnd");
	gameInstance->AddGoal("RenderStart");
	gameInstance->AddGoal("RenderSetup");
	gameInstance->AddGoal("RenderFinished");
	gameInstance->AddGoal("RenderEnd");
	gameInstance->AddGoal("FrameEnd");
	
	auto renderer = gameInstance->AddSystem<RenderSystem>("RenderSystem");

	RenderSystem::InitializeRendererInfo initialize_renderer_info;
	initialize_renderer_info.Window = &window;
	initialize_renderer_info.PipelineCacheResourceManager = GetResourceManager<PipelineCacheResourceManager>("PipelineCacheResourceManager");
	renderer->InitializeRenderer(initialize_renderer_info);

	auto* materialSystem = gameInstance->AddSystem<MaterialSystem>("MaterialSystem");
	{
		GTSL::Array<GTSL::Array<BindingType, 6>, 6> bindings(1);
		bindings[0].EmplaceBack(BindingType::COMBINED_IMAGE_SAMPLER);
		materialSystem->SetGlobalState(gameInstance, bindings);
	}

	auto* staticMeshRenderGroup = gameInstance->AddSystem<StaticMeshRenderGroup>("StaticMeshRenderGroup");
	
	{
		MaterialSystem::AddRenderGroupInfo addRenderGroupInfo;
		addRenderGroupInfo.Name = "StaticMeshRenderGroup";
		addRenderGroupInfo.Bindings.EmplaceBack();
		addRenderGroupInfo.Bindings[0].EmplaceBack(BindingType::UNIFORM_BUFFER_DYNAMIC);
		addRenderGroupInfo.Range.EmplaceBack();
		addRenderGroupInfo.Range[0].EmplaceBack(64);
		addRenderGroupInfo.Size.EmplaceBack();
		addRenderGroupInfo.Size.back().EmplaceBack(128);
		materialSystem->AddRenderGroup(gameInstance, addRenderGroupInfo);
	}

	gameInstance->AddSystem<CameraSystem>("CameraSystem");

	gameInstance->AddSystem<TextureSystem>("TextureSystem");

	auto* textSystem = gameInstance->AddSystem<TextSystem>("TextSystem");
	
	{
		auto* frameManager = gameInstance->AddSystem<FrameManager>("FrameManager");
		frameManager->AddAttachment(renderer, "Color", TextureFormat::RGBA_I8, TextureUses::COLOR_ATTACHMENT, TextureType::COLOR);
		frameManager->AddAttachment(renderer, "RenderDepth", TextureFormat::DEPTH32, TextureUses::DEPTH_STENCIL_ATTACHMENT, TextureType::DEPTH);
		frameManager->AddAttachment(renderer, "TextStencil", TextureFormat::STENCIL_8, TextureUses::DEPTH_STENCIL_ATTACHMENT, TextureType::STENCIL);

		GTSL::Array<FrameManager::AttachmentInfo, 6> attachments(3);
		attachments[0].Name = "Color";
		attachments[0].StartState = TextureLayout::UNDEFINED;
		attachments[0].EndState = TextureLayout::TRANSFER_SRC;
		attachments[0].Load = GAL::RenderTargetLoadOperations::CLEAR;
		attachments[0].Store = GAL::RenderTargetStoreOperations::STORE;

		attachments[1].Name = "RenderDepth";
		attachments[1].StartState = TextureLayout::UNDEFINED;
		attachments[1].EndState = TextureLayout::UNDEFINED;
		attachments[1].Load = GAL::RenderTargetLoadOperations::CLEAR;
		attachments[1].Store = GAL::RenderTargetStoreOperations::UNDEFINED;

		attachments[2].Name = "TextStencil";
		attachments[2].StartState = TextureLayout::UNDEFINED;
		attachments[2].EndState = TextureLayout::UNDEFINED;
		attachments[2].Load = GAL::RenderTargetLoadOperations::CLEAR;
		attachments[2].Store = GAL::RenderTargetStoreOperations::UNDEFINED;

		GTSL::Array<FrameManager::SubPassData, 6> subPasses;
		FrameManager::SubPassData geoRenderPass;
		geoRenderPass.DepthStencilAttachment.Name = "RenderDepth";
		geoRenderPass.DepthStencilAttachment.Layout = TextureLayout::DEPTH_STENCIL_ATTACHMENT;
		geoRenderPass.WriteAttachments.EmplaceBack("Color");
		geoRenderPass.WriteAttachmentsLayouts.EmplaceBack(TextureLayout::COLOR_ATTACHMENT);

		FrameManager::SubPassData textRenderPass;
		textRenderPass.DepthStencilAttachment.Name = "TextStencil";
		textRenderPass.DepthStencilAttachment.Layout = TextureLayout::STENCIL_ATTACHMENT;
		textRenderPass.WriteAttachments.EmplaceBack("Color");
		textRenderPass.WriteAttachmentsLayouts.EmplaceBack(TextureLayout::COLOR_ATTACHMENT);

		subPasses.EmplaceBack(geoRenderPass); subPasses.EmplaceBack(textRenderPass);
		
		frameManager->AddPass(renderer, "MainRenderPass", attachments, subPasses);
	}

	auto* renderOrchestrator = gameInstance->AddSystem<RenderOrchestrator>("RenderOrchestrator");
	renderOrchestrator->AddRenderGroup(gameInstance, "StaticMeshRenderGroup", staticMeshRenderGroup);
	renderOrchestrator->AddRenderGroup(gameInstance, "TextSystem", reinterpret_cast<RenderGroup*>(textSystem));
	
	window.ShowWindow();
}

void GameApplication::OnUpdate(const OnUpdateInfo& updateInfo)
{
	Application::OnUpdate(updateInfo);

	PROFILE;
	
	systemApplication.UpdateWindow(&window);
	
	switch (updateInfo.UpdateContext)
	{
		case UpdateContext::NORMAL:
		{		
			bool connected;
			GTSL::GamepadQuery::Update(gamepad, connected, 0);
		} break;
		
		case UpdateContext::BACKGROUND:
		{
			
		} break;
		
		default: break;
	}
}

void GameApplication::Shutdown()
{
	gameInstance->GetSystem<RenderSystem>("RenderSystem")->Wait();
	
	Application::Shutdown();
}

void GameApplication::SetupInputSources()
{
	RegisterMouse();
	RegisterKeyboard();
	RegisterControllers();
}

void GameApplication::RegisterMouse()
{
	inputManagerInstance->Register2DInputSource("MouseMove");

	auto mouse_move = [](const GTSL::Vector2 a)
	{
		Get()->GetInputManager()->Record2DInputSource("MouseMove", a);
	};

	inputManagerInstance->RegisterActionInputSource("LeftMouseButton");
	inputManagerInstance->RegisterActionInputSource("RightMouseButton");
	inputManagerInstance->RegisterActionInputSource("MiddleMouseButton");

	auto mouse_click = [](const GTSL::Window::MouseButton button, const bool buttonState)
	{
		switch (button)
		{
		case GTSL::Window::MouseButton::LEFT_BUTTON: Get()->GetInputManager()->RecordActionInputSource("LeftMouseButton", buttonState); break;
		case GTSL::Window::MouseButton::RIGHT_BUTTON: Get()->GetInputManager()->RecordActionInputSource("RightMouseButton", buttonState); break;
		case GTSL::Window::MouseButton::MIDDLE_BUTTON: Get()->GetInputManager()->RecordActionInputSource("MiddleMouseButton", buttonState); break;
		default:;
		}
	};

	inputManagerInstance->RegisterLinearInputSource("MouseWheel");

	auto mouse_wheel = [](const float value)
	{
		Get()->GetInputManager()->RecordLinearInputSource("MouseWheel", value);
	};

	window.SetOnMouseMoveDelegate(GTSL::Delegate<void(GTSL::Vector2)>::Create(mouse_move));
	window.SetOnMouseButtonClickDelegate(GTSL::Delegate<void(GTSL::Window::MouseButton, bool)>::Create(mouse_click));
	window.SetOnMouseWheelMoveDelegate(GTSL::Delegate<void(float32)>::Create(mouse_wheel));
}

void GameApplication::RegisterKeyboard()
{
	inputManagerInstance->RegisterCharacterInputSource("Keyboard");
	
	auto char_event = [](const uint32 ch)
	{
		Get()->GetInputManager()->RecordCharacterInputSource("Keyboard", ch);
	};
	window.SetOnCharEventDelegate(GTSL::Delegate<void(uint32)>::Create(char_event));

	inputManagerInstance->RegisterActionInputSource("Q_Key"); inputManagerInstance->RegisterActionInputSource("W_Key");
	inputManagerInstance->RegisterActionInputSource("E_Key"); inputManagerInstance->RegisterActionInputSource("R_Key");
	inputManagerInstance->RegisterActionInputSource("T_Key"); inputManagerInstance->RegisterActionInputSource("Y_Key");
	inputManagerInstance->RegisterActionInputSource("U_Key"); inputManagerInstance->RegisterActionInputSource("I_Key");
	inputManagerInstance->RegisterActionInputSource("O_Key"); inputManagerInstance->RegisterActionInputSource("P_Key");
	inputManagerInstance->RegisterActionInputSource("A_Key"); inputManagerInstance->RegisterActionInputSource("S_Key");
	inputManagerInstance->RegisterActionInputSource("D_Key"); inputManagerInstance->RegisterActionInputSource("F_Key");
	inputManagerInstance->RegisterActionInputSource("G_Key"); inputManagerInstance->RegisterActionInputSource("H_Key");
	inputManagerInstance->RegisterActionInputSource("J_Key"); inputManagerInstance->RegisterActionInputSource("K_Key");
	inputManagerInstance->RegisterActionInputSource("L_Key"); inputManagerInstance->RegisterActionInputSource("Z_Key");
	inputManagerInstance->RegisterActionInputSource("X_Key"); inputManagerInstance->RegisterActionInputSource("C_Key");
	inputManagerInstance->RegisterActionInputSource("V_Key"); inputManagerInstance->RegisterActionInputSource("B_Key");
	inputManagerInstance->RegisterActionInputSource("N_Key"); inputManagerInstance->RegisterActionInputSource("M_Key");
	inputManagerInstance->RegisterActionInputSource("0_Key"); inputManagerInstance->RegisterActionInputSource("1_Key");
	inputManagerInstance->RegisterActionInputSource("2_Key"); inputManagerInstance->RegisterActionInputSource("3_Key");
	inputManagerInstance->RegisterActionInputSource("4_Key"); inputManagerInstance->RegisterActionInputSource("5_Key");
	inputManagerInstance->RegisterActionInputSource("6_Key"); inputManagerInstance->RegisterActionInputSource("7_Key");
	inputManagerInstance->RegisterActionInputSource("8_Key"); inputManagerInstance->RegisterActionInputSource("9_Key");
	inputManagerInstance->RegisterActionInputSource("Backspace_Key");		inputManagerInstance->RegisterActionInputSource("Enter_Key");
	inputManagerInstance->RegisterActionInputSource("Supr_Key");			inputManagerInstance->RegisterActionInputSource("Tab_Key");
	inputManagerInstance->RegisterActionInputSource("CapsLock_Key");		inputManagerInstance->RegisterActionInputSource("Esc_Key");
	inputManagerInstance->RegisterActionInputSource("RightShift_Key");		inputManagerInstance->RegisterActionInputSource("LeftShift_Key");
	inputManagerInstance->RegisterActionInputSource("RightControl_Key");	inputManagerInstance->RegisterActionInputSource("LeftControl_Key");
	inputManagerInstance->RegisterActionInputSource("RightAlt_Key");		inputManagerInstance->RegisterActionInputSource("LeftAlt_Key");
	inputManagerInstance->RegisterActionInputSource("UpArrow_Key");			inputManagerInstance->RegisterActionInputSource("RightArrow_Key");
	inputManagerInstance->RegisterActionInputSource("DownArrow_Key");		inputManagerInstance->RegisterActionInputSource("LeftArrow_Key");
	inputManagerInstance->RegisterActionInputSource("SpaceBar_Key");
	inputManagerInstance->RegisterActionInputSource("Numpad0_Key"); inputManagerInstance->RegisterActionInputSource("Numpad1_Key");
	inputManagerInstance->RegisterActionInputSource("Numpad2_Key"); inputManagerInstance->RegisterActionInputSource("Numpad3_Key");
	inputManagerInstance->RegisterActionInputSource("Numpad4_Key"); inputManagerInstance->RegisterActionInputSource("Numpad5_Key");
	inputManagerInstance->RegisterActionInputSource("Numpad6_Key"); inputManagerInstance->RegisterActionInputSource("Numpad7_Key");
	inputManagerInstance->RegisterActionInputSource("Numpad8_Key"); inputManagerInstance->RegisterActionInputSource("Numpad9_Key");
	inputManagerInstance->RegisterActionInputSource("F1_Key"); inputManagerInstance->RegisterActionInputSource("F2_Key");
	inputManagerInstance->RegisterActionInputSource("F3_Key"); inputManagerInstance->RegisterActionInputSource("F4_Key");
	inputManagerInstance->RegisterActionInputSource("F5_Key"); inputManagerInstance->RegisterActionInputSource("F6_Key");
	inputManagerInstance->RegisterActionInputSource("F7_Key"); inputManagerInstance->RegisterActionInputSource("F8_Key");
	inputManagerInstance->RegisterActionInputSource("F9_Key"); inputManagerInstance->RegisterActionInputSource("F10_Key");
	inputManagerInstance->RegisterActionInputSource("F11_Key"); inputManagerInstance->RegisterActionInputSource("F12_Key");

	auto key_press = [](const GTSL::Window::KeyboardKeys key, const bool state, bool isFirstkeyOfType)
	{
		GTSL::Id64 id;
		switch (key)
		{
		case GTSL::Window::KeyboardKeys::Q: id = "Q_Key"; break;
		case GTSL::Window::KeyboardKeys::W: id = "W_Key"; break;
		case GTSL::Window::KeyboardKeys::E: id = "E_Key"; break;
		case GTSL::Window::KeyboardKeys::R: id = "R_Key"; break;
		case GTSL::Window::KeyboardKeys::T: id = "T_Key"; break;
		case GTSL::Window::KeyboardKeys::Y: id = "Y_Key"; break;
		case GTSL::Window::KeyboardKeys::U: id = "U_Key"; break;
		case GTSL::Window::KeyboardKeys::I: id = "I_Key"; break;
		case GTSL::Window::KeyboardKeys::O: id = "O_Key"; break;
		case GTSL::Window::KeyboardKeys::P: id = "P_Key"; break;
		case GTSL::Window::KeyboardKeys::A: id = "A_Key"; break;
		case GTSL::Window::KeyboardKeys::S: id = "S_Key"; break;
		case GTSL::Window::KeyboardKeys::D: id = "D_Key"; break;
		case GTSL::Window::KeyboardKeys::F: id = "F_Key"; break;
		case GTSL::Window::KeyboardKeys::G: id = "G_Key"; break;
		case GTSL::Window::KeyboardKeys::H: id = "H_Key"; break;
		case GTSL::Window::KeyboardKeys::J: id = "J_Key"; break;
		case GTSL::Window::KeyboardKeys::K: id = "K_Key"; break;
		case GTSL::Window::KeyboardKeys::L: id = "L_Key"; break;
		case GTSL::Window::KeyboardKeys::Z: id = "Z_Key"; break;
		case GTSL::Window::KeyboardKeys::X: id = "X_Key"; break;
		case GTSL::Window::KeyboardKeys::C: id = "C_Key"; break;
		case GTSL::Window::KeyboardKeys::V: id = "V_Key"; break;
		case GTSL::Window::KeyboardKeys::B: id = "B_Key"; break;
		case GTSL::Window::KeyboardKeys::N: id = "N_Key"; break;
		case GTSL::Window::KeyboardKeys::M: id = "M_Key"; break;
		case GTSL::Window::KeyboardKeys::Keyboard0: id = "0_Key"; break;
		case GTSL::Window::KeyboardKeys::Keyboard1: id = "1_Key"; break;
		case GTSL::Window::KeyboardKeys::Keyboard2: id = "2_Key"; break;
		case GTSL::Window::KeyboardKeys::Keyboard3: id = "3_Key"; break;
		case GTSL::Window::KeyboardKeys::Keyboard4: id = "4_Key"; break;
		case GTSL::Window::KeyboardKeys::Keyboard5: id = "5_Key"; break;
		case GTSL::Window::KeyboardKeys::Keyboard6: id = "6_Key"; break;
		case GTSL::Window::KeyboardKeys::Keyboard7: id = "7_Key"; break;
		case GTSL::Window::KeyboardKeys::Keyboard8: id = "8_Key"; break;
		case GTSL::Window::KeyboardKeys::Keyboard9: id = "9_Key"; break;
		case GTSL::Window::KeyboardKeys::Backspace: id = "Backspace_Key"; break;
		case GTSL::Window::KeyboardKeys::Enter: id = "Enter_Key"; break;
		case GTSL::Window::KeyboardKeys::Supr: id = "Supr_Key"; break;
		case GTSL::Window::KeyboardKeys::Tab: id = "Tab_Key"; break;
		case GTSL::Window::KeyboardKeys::CapsLock: id = "CapsLock_Key"; break;
		case GTSL::Window::KeyboardKeys::Esc: id = "Esc_Key"; break;
		case GTSL::Window::KeyboardKeys::RShift: id = "RightShift_Key"; break;
		case GTSL::Window::KeyboardKeys::LShift: id = "LeftShift_Key"; break;
		case GTSL::Window::KeyboardKeys::RControl: id = "RightControl_Key"; break;
		case GTSL::Window::KeyboardKeys::LControl: id = "LeftControl_Key"; break;
		case GTSL::Window::KeyboardKeys::Alt: id = "LeftAlt_Key"; break;
		case GTSL::Window::KeyboardKeys::AltGr: id = "RightAlt_Key"; break;
		case GTSL::Window::KeyboardKeys::UpArrow: id = "Up_Key"; break;
		case GTSL::Window::KeyboardKeys::RightArrow: id = "Right_Key"; break;
		case GTSL::Window::KeyboardKeys::DownArrow: id = "Down_Key"; break;
		case GTSL::Window::KeyboardKeys::LeftArrow: id = "Left_Key"; break;
		case GTSL::Window::KeyboardKeys::SpaceBar: id = "SpaceBar_Key"; break;
		case GTSL::Window::KeyboardKeys::Numpad0: id = "Numpad0_Key"; break;
		case GTSL::Window::KeyboardKeys::Numpad1: id = "Numpad1_Key"; break;
		case GTSL::Window::KeyboardKeys::Numpad2: id = "Numpad2_Key"; break;
		case GTSL::Window::KeyboardKeys::Numpad3: id = "Numpad3_Key"; break;
		case GTSL::Window::KeyboardKeys::Numpad4: id = "Numpad4_Key"; break;
		case GTSL::Window::KeyboardKeys::Numpad5: id = "Numpad5_Key"; break;
		case GTSL::Window::KeyboardKeys::Numpad6: id = "Numpad6_Key"; break;
		case GTSL::Window::KeyboardKeys::Numpad7: id = "Numpad7_Key"; break;
		case GTSL::Window::KeyboardKeys::Numpad8: id = "Numpad8_Key"; break;
		case GTSL::Window::KeyboardKeys::Numpad9: id = "Numpad9_Key"; break;
		case GTSL::Window::KeyboardKeys::F1: id = "F1_Key"; break;
		case GTSL::Window::KeyboardKeys::F2: id = "F2_Key"; break;
		case GTSL::Window::KeyboardKeys::F3: id = "F3_Key"; break;
		case GTSL::Window::KeyboardKeys::F4: id = "F4_Key"; break;
		case GTSL::Window::KeyboardKeys::F5: id = "F5_Key"; break;
		case GTSL::Window::KeyboardKeys::F6: id = "F6_Key"; break;
		case GTSL::Window::KeyboardKeys::F7: id = "F7_Key"; break;
		case GTSL::Window::KeyboardKeys::F8: id = "F8_Key"; break;
		case GTSL::Window::KeyboardKeys::F9: id = "F9_Key"; break;
		case GTSL::Window::KeyboardKeys::F10: id = "F10_Key";break;
		case GTSL::Window::KeyboardKeys::F11: id = "F11_Key"; break;
		case GTSL::Window::KeyboardKeys::F12: id = "F12_Key"; break;
		default: break;
		}

		if(isFirstkeyOfType)
		{
			Get()->GetInputManager()->RecordActionInputSource(id, state);
		}
	};
	
	window.SetOnKeyEventDelegate(GTSL::Delegate<void(GTSL::Window::KeyboardKeys, bool, bool)>::Create(key_press));
}

void GameApplication::RegisterControllers()
{
	inputManagerInstance->Register2DInputSource("LeftStick");
	inputManagerInstance->Register2DInputSource("RightStick");

	inputManagerInstance->RegisterActionInputSource("TopFrontButton");
	inputManagerInstance->RegisterActionInputSource("RightFrontButton");
	inputManagerInstance->RegisterActionInputSource("BottomFrontButton");
	inputManagerInstance->RegisterActionInputSource("LeftFrontButton");

	inputManagerInstance->RegisterActionInputSource("TopDPadButton");
	inputManagerInstance->RegisterActionInputSource("RightDPadButton");
	inputManagerInstance->RegisterActionInputSource("BottomDPadButton");
	inputManagerInstance->RegisterActionInputSource("LeftDPadButton");
	
	inputManagerInstance->RegisterActionInputSource("LeftStickButton");
	inputManagerInstance->RegisterActionInputSource("RightStickButton");
	
	inputManagerInstance->RegisterActionInputSource("LeftMenuButton");
	inputManagerInstance->RegisterActionInputSource("RightMenuButton");
	
	inputManagerInstance->RegisterActionInputSource("LeftHatButton");
	inputManagerInstance->RegisterActionInputSource("RightHatButton");
	
	inputManagerInstance->RegisterLinearInputSource("LeftTrigger");
	inputManagerInstance->RegisterLinearInputSource("RightTrigger");
	
	auto on_stick_move = [](GTSL::GamepadQuery::Side side, GTSL::Vector2 source)
	{
		switch(side)
		{
		case GTSL::GamepadQuery::Side::RIGHT: Get()->GetInputManager()->Record2DInputSource("RightStick", source); break;
		case GTSL::GamepadQuery::Side::LEFT: Get()->GetInputManager()->Record2DInputSource("LeftStick", source); break;
		default: break;
		}
	};

	auto on_trigger_change = [](GTSL::GamepadQuery::Side side, float32 source)
	{
		switch (side)
		{
		case GTSL::GamepadQuery::Side::RIGHT: Get()->GetInputManager()->RecordLinearInputSource("RightTrigger", source); break;
		case GTSL::GamepadQuery::Side::LEFT: Get()->GetInputManager()->RecordLinearInputSource("LeftTrigger", source); break;
		default: break;
		}
	};

	auto on_dpad_change = [](const GTSL::GamepadQuery::GamepadButtonPosition side, bool state)
	{
		switch (side)
		{
		case GTSL::GamepadQuery::GamepadButtonPosition::TOP: Get()->GetInputManager()->RecordActionInputSource("TopDPadButton", state); break;
		case GTSL::GamepadQuery::GamepadButtonPosition::RIGHT: Get()->GetInputManager()->RecordActionInputSource("RightDPadButton", state); break;
		case GTSL::GamepadQuery::GamepadButtonPosition::BOTTOM: Get()->GetInputManager()->RecordActionInputSource("BottomDPadButton", state); break;
		case GTSL::GamepadQuery::GamepadButtonPosition::LEFT: Get()->GetInputManager()->RecordActionInputSource("LeftDPadButton", state); break;
		default: break;
		}
	};

	auto on_hats_change = [](const GTSL::GamepadQuery::Side side, bool state)
	{
		switch (side)
		{
		case GTSL::GamepadQuery::Side::RIGHT: Get()->GetInputManager()->RecordActionInputSource("RightHatButton", state); break;
		case GTSL::GamepadQuery::Side::LEFT: Get()->GetInputManager()->RecordActionInputSource("LeftHatButton", state); break;
		default: break;
		}
	};

	auto on_menu_buttons_change = [](const GTSL::GamepadQuery::Side side, bool state)
	{
		switch (side)
		{
		case GTSL::GamepadQuery::Side::RIGHT: Get()->GetInputManager()->RecordActionInputSource("RightMenuButton", state); break;
		case GTSL::GamepadQuery::Side::LEFT: Get()->GetInputManager()->RecordActionInputSource("LeftMenuButton", state); break;
		default: break;
		}
	};

	auto on_front_buttons_change = [](const GTSL::GamepadQuery::GamepadButtonPosition side, bool state)
	{
		switch (side)
		{
		case GTSL::GamepadQuery::GamepadButtonPosition::TOP: Get()->GetInputManager()->RecordActionInputSource("TopFrontButton", state); break;
		case GTSL::GamepadQuery::GamepadButtonPosition::RIGHT: Get()->GetInputManager()->RecordActionInputSource("RightFrontButton", state); break;
		case GTSL::GamepadQuery::GamepadButtonPosition::BOTTOM: Get()->GetInputManager()->RecordActionInputSource("BottomFrontButton", state); break;
		case GTSL::GamepadQuery::GamepadButtonPosition::LEFT: Get()->GetInputManager()->RecordActionInputSource("LeftFrontButton", state); break;
		default: break;
		}
	};

	auto on_sticks_button_change = [](const GTSL::GamepadQuery::Side side, bool state)
	{
		switch (side)
		{
		case GTSL::GamepadQuery::Side::RIGHT: Get()->GetInputManager()->RecordActionInputSource("RightStickButton", state); break;
		case GTSL::GamepadQuery::Side::LEFT: Get()->GetInputManager()->RecordActionInputSource("LeftStickButton", state); break;
		default: break;
		}
	};
	
	gamepad.OnSticksMove = GTSL::Delegate<void(GTSL::GamepadQuery::Side, GTSL::Vector2)>::Create(on_stick_move);
	gamepad.OnDPadChange = GTSL::Delegate<void(GTSL::GamepadQuery::GamepadButtonPosition, bool)>::Create(on_dpad_change);
	gamepad.OnHatsChange = GTSL::Delegate<void(GTSL::GamepadQuery::Side, bool)>::Create(on_hats_change);
	gamepad.OnMenuButtonsChange = GTSL::Delegate<void(GTSL::GamepadQuery::Side, bool)>::Create(on_menu_buttons_change);
	gamepad.OnFrontButtonsChange = GTSL::Delegate<void(GTSL::GamepadQuery::GamepadButtonPosition, bool)>::Create(on_front_buttons_change);
	gamepad.OnSticksChange = GTSL::Delegate<void(GTSL::GamepadQuery::Side, bool)>::Create(on_sticks_button_change);
	gamepad.OnTriggersChange = GTSL::Delegate<void(GTSL::GamepadQuery::Side, float32)>::Create(on_trigger_change);
}

using namespace GTSL;

void GameApplication::onWindowResize(const GTSL::Extent2D& extent)
{
	auto* renderSystem = gameInstance->GetSystem<RenderSystem>("RenderSystem");
	auto* frameManager = gameInstance->GetSystem<FrameManager>("FrameManager");

	Array<TaskDependency, 10> taskDependencies = { { "RenderSystem", AccessType::READ_WRITE } };

	auto ext = extent;

	if (extent != 0 && extent != oldSize)
	{
		gameInstance->AddDynamicTask("RenderSystemWindowResize", Delegate<void(TaskInfo, Extent2D)>::Create<RenderSystem, &RenderSystem::OnResize>(renderSystem), taskDependencies, "FrameStart", "RenderStart", MoveRef(ext));
		gameInstance->AddDynamicTask("FrameManagerWindowResize", Delegate<void(TaskInfo, Extent2D)>::Create<FrameManager, &FrameManager::OnResize>(frameManager), taskDependencies, "FrameStart", "RenderStart", MoveRef(ext));
		oldSize = extent;
	}
	
}
