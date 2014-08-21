#pragma once

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreLogManager.h>
#include <OGRE/OgreRoot.h>
#include <OGRE/OgreViewport.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreConfigFile.h>
#include <OGRE/OgreWindowEventUtilities.h>

#include <OIS/OISEvents.h>
#include <OIS/OISInputManager.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/Renderer.h>

class BaseApplication : public Ogre::FrameListener, public Ogre::WindowEventListener, OIS::KeyListener, OIS::MouseListener {
public:
	BaseApplication(void);
	~BaseApplication(void);

    virtual void go(void);

protected:
	virtual bool setup();
    virtual bool configure(void);
    virtual void chooseSceneManager(void);
    virtual void createCamera(void);
    virtual void createFrameListener(void);
    virtual void createScene(void) = 0; // Override me!
    virtual void destroyScene(void);
    virtual void createViewports(void);
    virtual void setupResources(void);
    virtual void loadResources(void);
    virtual void setupGUI(void);

    // Ogre::FrameListener
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

    // OIS::KeyListener
	virtual bool keyPressed(const OIS::KeyEvent &keyEventRef);
	virtual bool keyReleased(const OIS::KeyEvent &keyEventRef);

    // OIS::MouseListener
	virtual bool mouseMoved(const OIS::MouseEvent &evt);
	virtual bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
	virtual bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

    virtual void windowClosed(Ogre::RenderWindow* rw);
    virtual CEGUI::MouseButton convertButton(OIS::MouseButtonID id);

    Ogre::Root* mRoot;
    Ogre::Camera* mCamera;
    Ogre::SceneManager* mSceneMgr;
    Ogre::RenderWindow* mRenderWnd;
    Ogre::Viewport* mViewport;
    Ogre::Log* mLog;
    Ogre::Timer* mTimer;

	OIS::InputManager* mInputMgr;
	OIS::Keyboard* mKeyboard;
	OIS::Mouse* mMouse;

    CEGUI::OgreRenderer* mGUIRenderer;
    CEGUI::Window* mRootWindow;

    bool bShutdown;
};

