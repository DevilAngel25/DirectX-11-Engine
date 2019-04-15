#include "Engine.h"

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	timer.Start();

	if (!render_window.Initialize(this, hInstance, window_title, window_class, width, height)) { return false; }

	if (!gfx.Initialize(render_window.GetHWND(), width, height)) { return false; }

	return true;
}

bool Engine::ProcessMessages()
{
	return render_window.ProcessMessages();
}

void Engine::Update()
{
	float dt = (float)timer.GetMillisecondsElapsed();
	timer.Restart();

	while (!keyboard.CharBufferIsEmpty())
	{
		unsigned char ch = keyboard.ReadChar();
	}

	while (!keyboard.KeyBufferIsEmpty())
	{
		KeyboardEvent kbe = keyboard.ReadKey();
		unsigned char keycode = kbe.GetKeyCode();
	}

	while (!mouse.EventBufferIsEmpty())
	{
		MouseEvent me = mouse.ReadEvent();
		if (mouse.IsRightDown())
		{
			if (me.GetType() == MouseEvent::EventType::RAW_MOVE)
			{
				gfx.camera3D.AdjustRotation((float)me.GetPosY() * 0.01f, (float)me.GetPosX() * 0.01f, 0.0f);
			}
		}
	}

	float cameraSpeed = 0.005f;

	if (keyboard.KeyIsPressed(VK_SHIFT))
	{
		cameraSpeed = 0.05f;
	}
	if (keyboard.KeyIsPressed('W'))
	{
		gfx.camera3D.AdjustPosition(gfx.camera3D.GetForwardVector() * cameraSpeed * dt);
	}
	if (keyboard.KeyIsPressed('S'))
	{
		gfx.camera3D.AdjustPosition(gfx.camera3D.GetBackwardVector() * cameraSpeed * dt);
	}
	if (keyboard.KeyIsPressed('A'))
	{
		gfx.camera3D.AdjustPosition(gfx.camera3D.GetLeftVector() * cameraSpeed * dt);
	}
	if (keyboard.KeyIsPressed('D'))
	{
		gfx.camera3D.AdjustPosition(gfx.camera3D.GetRightVector() * cameraSpeed * dt);
	}
	if (keyboard.KeyIsPressed(VK_SPACE))
	{
		gfx.camera3D.AdjustPosition(0.0f, cameraSpeed * dt, 0.0f);
	}
	if (keyboard.KeyIsPressed(VK_CONTROL))
	{
		gfx.camera3D.AdjustPosition(0.0f, -cameraSpeed * dt, 0.0f);
	}

	if (keyboard.KeyIsPressed('C'))
	{
		XMVECTOR lightPosition = gfx.camera3D.GetPositionVector();
		lightPosition += gfx.camera3D.GetForwardVector();
		gfx.pointLightOne.SetPosition(lightPosition);
		gfx.pointLightOne.SetRotation(gfx.camera3D.GetRotationFloat3());
	}
}

void Engine::RenderFrame()
{
	gfx.RenderFrame();
}
