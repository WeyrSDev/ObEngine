#include <TGUI/TGUI.hpp>

#include <Collision/PolygonalCollider.hpp>
#include <Debug/Console.hpp>
#include <Editor/CollidersEditActions.hpp>
#include <Editor/EditorGUI.hpp>
#include <Editor/Grid.hpp>
#include <Editor/MapEditor.hpp>
#include <Editor/MapEditorTools.hpp>
#include <Editor/EditorGlobalActions.hpp>
#include <Editor/SpriteEditActions.hpp>
#include <Input/InputManager.hpp>
#include <Input/KeyList.hpp>
#include <Graphics/DrawUtils.hpp>
#include <Network/Network.hpp>
#include <Scene/Scene.hpp>
#include <Script/GlobalState.hpp>
#include <System/Config.hpp>
#include <System/Cursor.hpp>
#include <System/Loaders.hpp>
#include <System/Window.hpp>
#include <Time/FramerateCounter.hpp>
#include <Time/FramerateManager.hpp>
#include <Transform/UnitVector.hpp>
#include <Triggers/TriggerDatabase.hpp>
#include <Utils/MathUtils.hpp>

namespace obe
{
    namespace Editor
    {
        void editMap(const std::string& mapName)
        {
            //Creating Window
            System::InitWindow(System::WindowContext::EditorWindow);

            Triggers::TriggerDatabase::GetInstance()->reg("TriggerDatabase");

            //Editor Triggers
            Triggers::TriggerGroupPtr editorTriggers(
                Triggers::TriggerDatabase::GetInstance()->createTriggerGroup("Global", "Editor"), 
                Triggers::TriggerGroupPtrRemover);

            //Editor Collider Triggers
            editorTriggers
                ->addTrigger("ColliderCreated")
                ->addTrigger("ColliderRemoved")
                ->addTrigger("ColliderPicked")
                ->addTrigger("ColliderMoved")
                ->addTrigger("ColliderReleased")
                ->addTrigger("ColliderPointCreated")
                ->addTrigger("ColliderPointRemoved")
                ->addTrigger("ColliderPointPicked")
                ->addTrigger("ColliderPointMoved")
                ->addTrigger("ColliderPointReleased");
            //Editor Various Triggers
            editorTriggers
                ->addTrigger("CameraMoved")
                ->addTrigger("GridCursorMoved")
                ->addTrigger("CursorMagnetized")
                ->addTrigger("GridToggled")
                ->addTrigger("GridSnapToggled")
                ->addTrigger("EditModeChanged")
                ->addTrigger("SceneSaved");
            //Editor Sprite Triggers
            editorTriggers
                ->addTrigger("SpriteZDepthChanged")
                ->addTrigger("SpriteLayerChanged")
                ->addTrigger("SpriteHandlePointPicked")
                ->addTrigger("SpriteHandlePointMoved")
                ->addTrigger("SpriteHandlePointReleased")
                ->addTrigger("SpriteCreated")
                ->addTrigger("SpriteSelect")
                ->addTrigger("SpriteMoved")
                ->addTrigger("SpriteUnselect")
                ->addTrigger("SpriteRemoved");

            //Game Triggers
            Triggers::TriggerGroupPtr gameTriggers(
                Triggers::TriggerDatabase::GetInstance()->createTriggerGroup("Global", "Game"),
                Triggers::TriggerGroupPtrRemover);
            gameTriggers
                ->addTrigger("Start")
                ->trigger("Start")
                ->addTrigger("End")
                ->addTrigger("Update")
                ->addTrigger("Render");

            //Console
            Debug::Console gameConsole;
            bool oldConsoleVisibility = false;
            std::vector<std::string> backupContexts;
            gameConsole.reg("Console");

            //Font
            sf::Font font;
            font.loadFromFile("Data/Fonts/arial.ttf");

            //Config
            vili::ComplexNode& gameConfig = System::Config->at("GameConfig");
            int scrollSensitive = gameConfig.at<vili::DataNode>("scrollSensibility");

            //Cursor
            System::Cursor cursor;
            cursor.reg("Cursor");

            //Scene Creation / Loading
            Scene::Scene scene;
            vili::ComplexNode sceneClipboard("Clipboard");
            //scene.getCamera()->bindView(window);
            Script::ScriptEngine["stream"] = gameConsole.createStream("Scene", true);
            Script::ScriptEngine.setErrorHandler([&gameConsole](int statuscode, const char* message)
            {
                gameConsole.pushMessage("LuaError", std::string("<Main> :: ") + message, sf::Color::Red);
                Debug::Log->error("<LuaError>({0}) : {1}", statuscode, message);
            });
            scene.reg("Scene");

            //Socket
            Network::NetworkHandler networkHandler;

            //Keybinding
            Input::InputManager inputManager;
            inputManager.reg("InputManager");
            inputManager.configure(System::Config.at("KeyBinding"));
            inputManager
                .addContext("game")
                .addContext("mapEditor")
                .addContext("gameConsole");

            //Editor Grid
            EditorGrid editorGrid(32, 32);
            editorGrid.reg("Grid");
            inputManager.getAction("MagnetizeUp").setRepeat(200);
            inputManager.getAction("MagnetizeDown").setRepeat(200);
            inputManager.getAction("MagnetizeLeft").setRepeat(200);
            inputManager.getAction("MagnetizeRight").setRepeat(200);

            //GUI
            sf::Event event;
            tgui::Gui gui(System::MainWindow);
            gui.setFont("Data/Fonts/weblysleekuil.ttf");
            tgui::Panel::Ptr mainPanel = tgui::Panel::create();
            GUI::init();
            int saveEditMode = -1;
            gui.add(mainPanel);
            mainPanel->setSize("100%", "100%");

            GUI::buildEditorMenu(mainPanel);
            GUI::buildObjectCreationMenu(mainPanel);

            tgui::Panel::Ptr editorPanel = gui.get<tgui::Panel>("editorPanel");
            tgui::Panel::Ptr mapPanel = gui.get<tgui::Panel>("mapPanel");
            tgui::Panel::Ptr settingsPanel = gui.get<tgui::Panel>("settingsPanel");
            tgui::Panel::Ptr spritesPanel = gui.get<tgui::Panel>("spritesPanel");
            tgui::Panel::Ptr objectsPanel = gui.get<tgui::Panel>("objectsPanel");
            tgui::Panel::Ptr requiresPanel = gui.get<tgui::Panel>("requiresPanel");
            tgui::Scrollbar::Ptr spritesScrollbar = gui.get<tgui::Scrollbar>("spritesScrollbar");
            tgui::Scrollbar::Ptr objectsScrollbar = gui.get<tgui::Scrollbar>("objectsScrollbar");

            GUI::buildToolbar(mainPanel, editorPanel, scene);

            tgui::ComboBox::Ptr editMode = gui.get<tgui::ComboBox>("editMode");

            GUI::buildEditorMapMenu(mapPanel, scene);
            GUI::buildEditorSettingsMenu(settingsPanel, editorGrid, cursor, editMode, scene);
            GUI::buildEditorSpritesMenu(spritesPanel, spritesScrollbar, scene);
            GUI::buildEditorObjectsMenu(objectsPanel, requiresPanel, objectsScrollbar, scene);

            tgui::EditBox::Ptr cameraSizeInput = gui.get<tgui::EditBox>("cameraSizeInput");
            tgui::EditBox::Ptr cameraPositionXInput = gui.get<tgui::EditBox>("cameraPositionXInput");
            tgui::EditBox::Ptr cameraPositionYInput = gui.get<tgui::EditBox>("cameraPositionYInput");

            tgui::CheckBox::Ptr enableGridCheckbox = gui.get<tgui::CheckBox>("enableGridCheckbox");
            tgui::CheckBox::Ptr snapGridCheckbox = gui.get<tgui::CheckBox>("snapGridCheckbox");
            tgui::EditBox::Ptr mapNameInput = gui.get<tgui::EditBox>("mapNameInput");
            tgui::Label::Ptr savedLabel = gui.get<tgui::Label>("savedLabel");
            tgui::Label::Ptr infoLabel = gui.get<tgui::Label>("infoLabel");
            tgui::CheckBox::Ptr displayFramerateCheckbox = gui.get<tgui::CheckBox>("displayFramerateCheckbox");
            tgui::CheckBox::Ptr saveCameraPositionCheckbox = gui.get<tgui::CheckBox>("saveCameraPositionCheckbox");

            //Map Editor
            Graphics::LevelSprite* hoveredSprite = nullptr;
            Graphics::LevelSprite* selectedSprite = nullptr;
            Graphics::LevelSpriteHandlePoint* selectedHandlePoint = nullptr;
            int selectedSpriteOffsetX = 0;
            int selectedSpriteOffsetY = 0;
            int cameraSpeed = Transform::UnitVector::Screen.h;
            int currentLayer = 1;
            Collision::PolygonalCollider* selectedMasterCollider = nullptr;
            int colliderPtGrabbed = -1;
            bool masterColliderGrabbed = false;
            sf::Text sprInfo;
            sprInfo.setFont(font);
            sprInfo.setCharacterSize(16);
            sprInfo.setFillColor(sf::Color::White);
            sf::RectangleShape sprInfoBackground(sf::Vector2f(100, 160));
            sprInfoBackground.setFillColor(sf::Color(0, 0, 0, 200));
            double waitForMapSaving = -1;
            double saveCamPosX = 0;
            double saveCamPosY = 0;
            Transform::Units editorUnit = Transform::Units::WorldUnits;

            //Framerate / DeltaTime
            Time::FPSCounter fps;
            fps.loadFont(font);
            Time::FramerateManager framerateManager(gameConfig);

            scene.loadFromFile(mapName);

            mapNameInput->setText(scene.getLevelName());
            cameraSizeInput->setText(std::to_string(scene.getCamera()->getSize().y / 2));
                
            //Connect InputManager Actions
            connectSaveActions(editorTriggers.get(), inputManager, mapName, scene, waitForMapSaving, savedLabel, saveCameraPositionCheckbox);
            connectCamMovementActions(editorTriggers.get(), inputManager, scene, cameraSpeed, framerateManager);
            connectGridActions(editorTriggers.get(), inputManager, enableGridCheckbox, snapGridCheckbox, cursor, editorGrid);
            connectMenuActions(inputManager, editMode, editorPanel);
            connectSpriteLayerActions(editorTriggers.get(), inputManager, selectedSprite, scene, currentLayer);
            connectSpriteActions(editorTriggers.get(), inputManager, hoveredSprite, selectedSprite, selectedHandlePoint,
                scene, cursor, editorGrid, selectedSpriteOffsetX, selectedSpriteOffsetY, sprInfo, sprInfoBackground, editorUnit);
            connectCollidersActions(editorTriggers.get(), inputManager, scene, cursor, colliderPtGrabbed, selectedMasterCollider, masterColliderGrabbed);
            connectGameConsoleActions(inputManager, gameConsole);
            inputManager.getAction("ExitEditor").connect([](const Input::InputActionEvent& event)
            {
                System::MainWindow.close();
            });

            auto editModeCallback = [&editorTriggers, &inputManager, editMode]()
            {
                editorTriggers->pushParameter("EditModeChanged", "mode", editMode->getSelectedItem());
                editorTriggers->trigger("EditModeChanged");
                if (editMode->getSelectedItem() == "LevelSprites")
                {
                    inputManager.addContext("spriteEditing");
                }
                else
                {
                    inputManager.removeContext("spriteEditing");
                }
                if (editMode->getSelectedItem() == "Collisions")
                {
                    inputManager.addContext("colliderEditing");
                }
                else
                {
                    inputManager.removeContext("colliderEditing");
                }
            };

            editMode->connect("itemselected", editModeCallback);

            GUI::calculateFontSize();
            GUI::applyFontSize(mainPanel);

            //scene.setUpdateState(false);

            //Game Starts
            while (System::MainWindow.isOpen())
            {
                framerateManager.update();

                gameTriggers->pushParameter("Update", "dt", framerateManager.getGameSpeed());
                gameTriggers->trigger("Update");

                if (framerateManager.doRender())
                    gameTriggers->trigger("Render");

                if (waitForMapSaving >= 0)
                {
                    waitForMapSaving += framerateManager.getDeltaTime();
                    if (waitForMapSaving > 1 && waitForMapSaving < 2)
                    {
                        savedLabel->hideWithEffect(tgui::ShowAnimationType::SlideFromTop, sf::Time(sf::seconds(0.5)));
                        waitForMapSaving = 2;
                    }
                    else if (waitForMapSaving > 3)
                        waitForMapSaving = -1;
                }

                bool drawFPS = displayFramerateCheckbox->isChecked();

                if (editorPanel->isVisible() && saveEditMode < 0)
                {
                    saveEditMode = editMode->getSelectedItemIndex();
                    editMode->setSelectedItemByIndex(3);
                }
                else if (!editorPanel->isVisible() && saveEditMode > 0)
                {
                    editMode->setSelectedItemByIndex(saveEditMode);
                    saveEditMode = -1;
                }

                Transform::UnitVector pixelCamera = scene.getCamera()->getPosition().to<Transform::Units::WorldPixels>();
                //Updates

                if (oldConsoleVisibility != gameConsole.isVisible())
                {
                    if (oldConsoleVisibility)
                    {
                        inputManager.clearContexts();
                        for (std::string& context : backupContexts)
                        {
                            inputManager.addContext(context);
                        }
                        backupContexts.clear();
                    }
                    else
                    {
                        backupContexts = inputManager.getContexts();
                        inputManager.setContext("gameConsole");
                    }
                    oldConsoleVisibility = gameConsole.isVisible();
                }

                //Sprite Editing
                if (editMode->getSelectedItem() == "LevelSprites")
                {
                    scene.enableShowCollision(true, true, false, false);

                    if (hoveredSprite == nullptr)
                    {
                        hoveredSprite = scene.getLevelSpriteByPosition(cursor.getPosition(), -pixelCamera, currentLayer);
                        if (hoveredSprite != nullptr && hoveredSprite != selectedSprite)
                        {
                            hoveredSprite = scene.getLevelSpriteByPosition(cursor.getPosition(), -pixelCamera, currentLayer);
                            hoveredSprite->setColor(sf::Color(0, 255, 255));
                            std::string sprInfoStr = "Hovered Sprite : \n";
                            sprInfoStr += "    Id : " + hoveredSprite->getId() + "\n";
                            sprInfoStr += "    Name : " + hoveredSprite->getPath() + "\n";
                            sprInfoStr += "    Pos : " + std::to_string(hoveredSprite->getPosition().to(editorUnit).x) + "," 
                            + std::to_string(hoveredSprite->getPosition().to(editorUnit).y) + "\n";
                            sprInfoStr += "    Size : " + std::to_string(hoveredSprite->getSize().to(editorUnit).x) 
                            + "," + std::to_string(hoveredSprite->getSize().to(editorUnit).y) + "\n";
                            sprInfoStr += "    Rot : " + std::to_string(hoveredSprite->getRotation()) + "\n";
                            sprInfoStr += "    Layer / Z : " + std::to_string(hoveredSprite->getLayer()) + "," + std::to_string(hoveredSprite->getZDepth()) + "\n";
                            sprInfo.setString(sprInfoStr);
                            sprInfoBackground.setSize(sf::Vector2f(sprInfo.getGlobalBounds().width + 20, sprInfo.getGlobalBounds().height - 10));
                        }
                        else
                            hoveredSprite == nullptr;
                    }
                    else if (hoveredSprite != nullptr)
                    {
                        sprInfoBackground.setPosition(cursor.getX() + 40, cursor.getY());
                        sprInfo.setPosition(cursor.getX() + 50, cursor.getY());
                        bool outHover = false;
                        Graphics::LevelSprite* testHoverSprite = scene.getLevelSpriteByPosition(cursor.getPosition(), -pixelCamera, currentLayer);
                        if (testHoverSprite != hoveredSprite)
                            outHover = true;
                        if (outHover)
                        {
                            if (hoveredSprite != selectedSprite)
                                hoveredSprite->setColor(sf::Color::White);
                            hoveredSprite = nullptr;
                            sprInfo.setString("");
                        }
                    }                      
                }
                else
                {
                    if (selectedSprite != nullptr)
                    {
                        selectedSprite->setColor(sf::Color::White);
                        selectedSprite->setSelected(false);
                    }
                    if (hoveredSprite != nullptr)
                    {
                        hoveredSprite->setColor(sf::Color::White);
                    }
                    selectedSprite = nullptr;
                    hoveredSprite = nullptr;
                    selectedSpriteOffsetX = 0;
                    selectedSpriteOffsetY = 0;
                    sprInfo.setString("");
                }

                //Collision Edition
                if (editMode->getSelectedItem() == "Collisions")
                {
                    Transform::UnitVector cursCoord(cursor.getConstrainedX() + pixelCamera.x, cursor.getConstrainedY() + pixelCamera.y);

                    scene.enableShowCollision(true, true, true, true);
                    if (selectedMasterCollider != nullptr)
                    {
                        selectedMasterCollider->clearHighlights();
                        selectedMasterCollider->highlightLine(selectedMasterCollider->findClosestLine(cursCoord));
                    }     
                }

                //GUI Update
                infoLabel->setText(
                    "Cursor : (" 
                    + std::to_string(cursor.getX()) 
                    + ", " 
                    + std::to_string(cursor.getY()) 
                    + ")" 
                    + std::string("   Camera : (") 
                    + std::to_string(int(scene.getCamera()->getPosition(Transform::Referencial::TopLeft).to<Transform::Units::WorldPixels>().x)) 
                    + ", " 
                    + std::to_string(int(scene.getCamera()->getPosition(Transform::Referencial::TopLeft).to<Transform::Units::WorldPixels>().y))
                    + ")" 
                    + std::string("   Sum : (") 
                    + std::to_string(int(scene.getCamera()->getPosition(Transform::Referencial::TopLeft).to<Transform::Units::WorldPixels>().x)
                        + int(cursor.getX())) 
                    + ", " + std::to_string(int(scene.getCamera()->getPosition(Transform::Referencial::TopLeft).to<Transform::Units::WorldPixels>().y)
                        + int(cursor.getY())) 
                    + ")" 
                    + std::string("   Layer : ") 
                    + std::to_string(currentLayer)
                );

                //Events
                scene.update(framerateManager.getGameSpeed());
                Triggers::TriggerDatabase::GetInstance()->update();
                inputManager.update();
                cursor.update();
                if (drawFPS) fps.uTick();
                if (drawFPS && framerateManager.doRender()) fps.tick();

                //Triggers Handling
                networkHandler.handleTriggers();
                //cursor.handleTriggers();
                inputManager.handleTriggers();

                while (System::MainWindow.pollEvent(event))
                {
                    switch (event.type)
                    {
                    case sf::Event::Closed:
                        System::MainWindow.close();
                        break;
                    case sf::Event::Resized:
                        Transform::UnitVector::Screen.w = event.size.width;
                        Transform::UnitVector::Screen.h = event.size.height;
                        System::MainWindow.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
                        gui.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
                        GUI::calculateFontSize();
                        GUI::applyFontSize(mainPanel);
                        GUI::applyScrollbarMaxValue(mainPanel);
                        break;
                    case sf::Event::JoystickConnected:
                        Input::SetGamepadList();
                        break;
                    case sf::Event::JoystickDisconnected:
                        Input::SetGamepadList();
                        break;
                    case sf::Event::TextEntered:
                        if (gameConsole.isVisible())
                            gameConsole.inputKey(event.text.unicode);
                        break;
                    case sf::Event::GainedFocus:
                        inputManager.setEnabled(true);
                        break;
                    case sf::Event::LostFocus:
                        inputManager.setEnabled(false);
                        break;
                    case sf::Event::MouseWheelMoved:
                        if (event.mouseWheel.delta >= scrollSensitive)
                        {
                            scene.getCamera()->scale(0.9);
                            gameConsole.scroll(-1);
                        }
                        else if (event.mouseWheel.delta <= -scrollSensitive)
                        {
                            scene.getCamera()->scale(1.1);
                            gameConsole.scroll(1);
                        }
                        cameraSizeInput->setText(std::to_string(scene.getCamera()->getSize().y / 2));
                        break;
                    default: ;
                    }
                    gui.handleEvent(event);
                }
                //Draw Everything Here
                if (framerateManager.doRender())
                {
                    if (saveCamPosX != scene.getCamera()->getPosition(Transform::Referencial::TopLeft).x || saveCamPosY != scene.getCamera()->getPosition(Transform::Referencial::TopLeft).y)
                    {
                        saveCamPosX = scene.getCamera()->getPosition(Transform::Referencial::TopLeft).x;
                        saveCamPosY = scene.getCamera()->getPosition(Transform::Referencial::TopLeft).y;
                        cameraPositionXInput->setText(std::to_string(saveCamPosX));
                        cameraPositionYInput->setText(std::to_string(saveCamPosY));
                    }

                    System::MainWindow.clear(Graphics::Utils::clearColor);
                    scene.display();
                    sf::Color magenta = sf::Color::Magenta;
                    if (selectedHandlePoint != nullptr)
                        Graphics::Utils::drawPoint(selectedHandlePoint->m_dp.x, selectedHandlePoint->m_dp.y, 3, magenta);
                    pixelCamera = scene.getCamera()->getPosition().to<Transform::Units::WorldPixels>(); // Do it once (Grid Draw Offset) <REVISION>
                    //Show Collision
                    if (editMode->getSelectedItem() == "Collisions")
                        scene.enableShowCollision(true);
                    else
                        scene.enableShowCollision(false);
                    if (editorGrid.isEnabled())
                        editorGrid.draw(cursor, pixelCamera.x, pixelCamera.y);
                    //HUD & GUI
                    if (sprInfo.getString() != "")
                    {
                        System::MainWindow.draw(sprInfoBackground);
                        System::MainWindow.draw(sprInfo);
                    }
                    gui.draw();
                    if (drawFPS)
                        fps.draw();

                    //Console
                    if (gameConsole.isVisible())
                        gameConsole.display();

                    System::MainWindow.display();
                }
            }
            gameTriggers->trigger("End");
            Triggers::TriggerDatabase::GetInstance()->update();
            scene.update(framerateManager.getGameSpeed());

            System::MainWindow.close();
            gui.removeAllWidgets();
        }
    }
}
