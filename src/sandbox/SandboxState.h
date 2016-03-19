/*
 * Dawn Engine
 * Written by David Avedissian (c) 2012-2016 (git@davedissian.com)
 */
#pragma once

using dw::uint;
using dw::String;

enum StateID
{
	S_SANDBOX = dw::S_USER_ID_BEGIN
};

// The state where the universe can be explored. Useful for debugging the planetary engine
class SandboxState : public dw::State
{
public:
    SandboxState(dw::Engine* engine);
    virtual ~SandboxState();

    void HandleEvent(dw::EventDataPtr eventData);

    // Inherited from State
    virtual void Enter();
    virtual void Exit();
    virtual void Update(float dt);
    virtual uint GetID() const { return S_SANDBOX; }
    virtual String GetName() const { return "SandboxState"; }

private:
	dw::Engine* mEngine;
	dw::DefaultCamera* mCamera;
	dw::Layout* mData;

    double mTime;
    double mDeltaTime;
	dw::SystemBody* mTrackedObject;

    void UpdateUI(float speed);
};
