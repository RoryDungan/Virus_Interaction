#pragma once

#include "BaseApplication.hpp"
#include "World.hpp"

class Simulation : public BaseApplication 
{
public:
	Simulation(void);
	virtual ~Simulation(void);

protected:
    virtual void createScene();
    virtual void createFrameListener();

    // Ogre::FrameListener
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

    // OIS::KeyListener
	virtual bool keyPressed(const OIS::KeyEvent &keyEventRef);
	virtual bool keyReleased(const OIS::KeyEvent &keyEventRef);

    // OIS::MouseListener
	virtual bool mouseMoved(const OIS::MouseEvent &evt);
	virtual bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
	virtual bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

private: 
    bool quitEvent(const CEGUI::EventArgs& evt);
    bool resetEvent(const CEGUI::EventArgs& evt);
    bool tempChangedEvent(const CEGUI::EventArgs& evt);
    bool phChangedEvent(const CEGUI::EventArgs& evt);
    bool numCellsChangedEvent(const CEGUI::EventArgs& evt);

    void calculateCameraMovement();
    void selectLifeForm(LifeForm* lifeForm);
    void updateInfoBox();

    World* mWorld;
    LifeForm* mSelectedLifeForm;
    Ogre::RaySceneQuery* mRaySceneQuery;

    CEGUI::Window* mHud;

    Ogre::Vector3 mCameraMovement;
    static const float CAMERA_MOVEMENT_SPEED;
    static const OIS::KeyCode KEY_CAM_UP;
    static const OIS::KeyCode KEY_CAM_LEFT;
    static const OIS::KeyCode KEY_CAM_RIGHT;
    static const OIS::KeyCode KEY_CAM_DOWN;
};

