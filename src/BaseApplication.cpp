#include "BaseApplication.hpp"

using namespace Ogre;

BaseApplication::BaseApplication(void) :
    mRoot(0),
    mRenderWnd(0),
    mViewport(0),
    mLog(0),
    mTimer(0),
    mInputMgr(0),
    bShutdown(false)
{}

BaseApplication::~BaseApplication(void) {
    mLog->logMessage("Shutting down Ogre...");

    windowClosed(mRenderWnd);
}

bool BaseApplication::configure(void) {
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->showConfigDialog()) {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mRenderWnd = mRoot->initialise(true, "Virus Interaction Simulation");

        return true;
    }
    else {
        return false;
    }
}

void BaseApplication::chooseSceneManager(void) {
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
}

void BaseApplication::createCamera(void) {
    mCamera = mSceneMgr->createCamera("MainCamera");
}

void BaseApplication::createFrameListener(void) {
    // Set up input manager
    mLog->logMessage("Initialising OIS...");
    unsigned long hWnd = 0;
    OIS::ParamList paramList;
    mRenderWnd->getCustomAttribute("WINDOW", &hWnd);
    paramList.insert(OIS::ParamList::value_type("WINDOW", Ogre::StringConverter::toString(hWnd)));

    mInputMgr = OIS::InputManager::createInputSystem(paramList);

    mKeyboard = static_cast<OIS::Keyboard*>(mInputMgr->createInputObject(OIS::OISKeyboard, true));
    mMouse = static_cast<OIS::Mouse*>(mInputMgr->createInputObject(OIS::OISMouse, true));

    mKeyboard->setEventCallback(this);
    mMouse->setEventCallback(this);

    mMouse->getMouseState().width = mRenderWnd->getWidth();
    mMouse->getMouseState().height = mRenderWnd->getHeight();
    
	Ogre::WindowEventUtilities::addWindowEventListener(mRenderWnd, this);

    mRoot->addFrameListener(this);
}

void BaseApplication::destroyScene(void) {
    //stub
}

void BaseApplication::createViewports(void) {
    mViewport = mRenderWnd->addViewport(mCamera);
    mViewport->setBackgroundColour(ColourValue(0.0f, 0.0f, 0.0f));

    // Alter camera aspect ration to match the viewport
    mCamera->setAspectRatio(Real(mViewport->getActualWidth()) / Real(mViewport->getActualHeight()));
}

void BaseApplication::setupResources(void) {
    // Set up resource groups
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation("media", "FileSystem");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation("media/fonts", "FileSystem", "Fonts");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation("media/imagesets", "FileSystem", "Imagesets");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation("media/schemes", "FileSystem", "Schemes");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation("media/looknfeel", "FileSystem", "LookNFeel");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation("media/layouts", "FileSystem", "Layouts");
}

void BaseApplication::loadResources(void) {
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void BaseApplication::go(void) {
    if(!setup()) {
        return;
	}

    mRoot->startRendering();

    // Clean up
    destroyScene();
}

bool BaseApplication::setup(void) {
    // Set up log file
    Ogre::LogManager* logMgr = new Ogre::LogManager();
    mLog = logMgr->createLog("Ogre.log", true, true, false);
#ifdef _DEBUG
    mLog->setDebugOutputEnabled(true);
#endif

    // Set up Ogre root and load plugins
    mRoot = new Ogre::Root();
#ifdef _DEBUG
    mRoot->loadPlugin("RenderSystem_GL_d");
#else
    mRoot->loadPlugin("RenderSystem_GL");
#endif

    setupResources();

    if(!configure()) {
        return false;
	}

    chooseSceneManager();
    createCamera();
    createViewports();

    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

    // Load resources
    loadResources();

    // Set up GUI
    setupGUI();

    // Create the scene
    createScene();

    createFrameListener();

    return true;
}

bool BaseApplication::frameRenderingQueued(const Ogre::FrameEvent& evt) {
    if(bShutdown || mRenderWnd->isClosed()) {
        return false;
	}

    mKeyboard->capture();
    mMouse->capture();

    // Inject timestamps to CEGUI System
    CEGUI::System::getSingleton().injectTimePulse(evt.timeSinceLastFrame);

    return true;
}

void BaseApplication::setupGUI(void) {
    // Set up CEGUI
    mGUIRenderer = &CEGUI::OgreRenderer::bootstrapSystem();

    CEGUI::ImageManager::setImagesetDefaultResourceGroup("Imagesets");
    CEGUI::Font::setDefaultResourceGroup("Fonts");
    CEGUI::Scheme::setDefaultResourceGroup("Schemes");
    CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
    CEGUI::WindowManager::setDefaultResourceGroup("Layouts");

    CEGUI::SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
    CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage("TaharezLook/MouseArrow");

    mRootWindow = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "Root");
    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(mRootWindow);
}

bool BaseApplication::keyPressed(const OIS::KeyEvent& keyEventRef) {
    CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
    context.injectKeyDown((CEGUI::Key::Scan)keyEventRef.key);
    context.injectChar((CEGUI::Key::Scan)keyEventRef.text);

    return true;
}

bool BaseApplication::keyReleased(const OIS::KeyEvent& keyEventRef) {
    CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyUp((CEGUI::Key::Scan)keyEventRef.key);

    return true;
}

bool BaseApplication::mouseMoved(const OIS::MouseEvent& evt) {
    CEGUI::GUIContext &context = CEGUI::System::getSingleton().getDefaultGUIContext();
    context.injectMouseMove((float)evt.state.X.rel, (float)evt.state.Y.rel);
    // Scroll wheel.
    if (evt.state.Z.rel) {
        context.injectMouseWheelChange((float)evt.state.Z.rel / 120.0f);
    }

    return true;
}

bool BaseApplication::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id) {
    CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(convertButton(id));

    return true;
}

bool BaseApplication::mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id) {
    CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonUp(convertButton(id));

    return true;
}

// Unattach OIS before window shutdown
void BaseApplication::windowClosed(Ogre::RenderWindow* rw) {
    if(rw == mRenderWnd) {
        if(mInputMgr) {
            mInputMgr->destroyInputObject(mMouse);
            mInputMgr->destroyInputObject(mKeyboard);

            OIS::InputManager::destroyInputSystem(mInputMgr);
            mInputMgr = 0;
		}
	}
}

CEGUI::MouseButton BaseApplication::convertButton(OIS::MouseButtonID id) {
    switch(id) {
	case OIS::MB_Left:
        return CEGUI::LeftButton;

	case OIS::MB_Right:
        return CEGUI::RightButton;

	case OIS::MB_Middle:
        return CEGUI::MiddleButton;

	default:
        return CEGUI::LeftButton;
	}
}