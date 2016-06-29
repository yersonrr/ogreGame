#include "Ogre\ExampleApplication.h"
#include "math.h"


AnimationState * wheelState[4];
AnimationState * asteroidState[9];
char* wheelName[] = {"wheel1_anim", "wheel2_anim", "wheel3_anim", "wheel4_anim"};
Ogre::SceneNode* _nodeRueda[4];
Ogre::SceneNode* _nodeCoins[20];
Ogre::Entity* _entCoins[20];
bool isTakenCoin[20];
bool is_accelerating = false;
float time_accelerating = 0.0f;

Ogre::AnimationState* animSphere;
Ogre::AnimationState* animSphere2;
Ogre::AnimationState* animAsteroid;
Ogre::AnimationState* animAsteroid2;
Ogre::AnimationState* animAsteroid3;
Ogre::AnimationState* animAsteroid4;
Ogre::AnimationState* animAsteroid9;
Ogre::AnimationState* animAsteroid10;
Ogre::AnimationState* animAsteroid11;
Ogre::AnimationState* animAsteroid12;

std::vector<Ogre::SceneNode *> obstacles;
std::vector<Ogre::SceneNode *> coins;

class FrameListenerClase : public Ogre::FrameListener{

private:
	Ogre::SceneNode* _node;
	Ogre::AnimationState* _anim;
	OIS::InputManager* _man;
	OIS::Keyboard* _key;
	Ogre::Camera* _cam;

public:
	FrameListenerClase(Ogre::SceneNode* node, Ogre::Entity* entOgre01, Ogre::Camera* cam, RenderWindow* win){
		//Configuracion para captura de teclado y mouse 
		size_t windowHnd = 0;
		std::stringstream windowHndStr;
		win->getCustomAttribute("WINDOW",&windowHnd);
		windowHndStr << windowHnd;

		OIS::ParamList pl;
		pl.insert(std::make_pair(std::string("WINDOW"),windowHndStr.str()));

		//eventos
		_man = OIS::InputManager::createInputSystem(pl);
		_key = static_cast<OIS::Keyboard*>(_man->createInputObject(OIS::OISKeyboard,false));
		_cam = cam;
		_node = node;
	}

	~FrameListenerClase(){
		_man->destroyInputObject(_key);
		OIS::InputManager::destroyInputSystem(_man);
	}

	bool collides_any_wall(Ogre::SceneNode *node) {
		Ogre::Vector3 position = node->_getDerivedPosition();
		if (position.z < 420){
			if(position.x <= -128.5 || position.x >= 128.5) return true;
		} else if (position.z < 2355){
			if(position.x <= -205.5 || position.x >= 205.5) return true;
		} else if (position.z < 4955 && position.z > 2900){
			if(position.x <= -28 || position.x >= 28) return true;
		} else if (position.z > 5500){
			if(position.x <= -205.5 || position.x >= 205.5) return true;
		} else if (position.z > 2300 && position.z < 2900 && position.x >= 0){
			if(position.z > -3.76*position.x + 3108.33) return true;
		} else if (position.z > 2300 && position.z < 2900 && position.x <= 0){
			if(position.z > 3.76*position.x + 3050) return true;
		} else if (position.z > 4900 && position.z < 5501 && position.x >= 0){
			if(position.z < 3.53*position.x + 4780) return true;
		} else if (position.z > 4900 && position.z < 5501 && position.x <= 0){
			if(position.z < -3.53*position.x + 4780) return true;
		}
		return false;
	}

	bool collides(Ogre::SceneNode *node1, Ogre::SceneNode *node2) {
		// determine if there is a sphere to sphere collision
		Ogre::Vector3 diff = node1->_getDerivedPosition() - node2->_getDerivedPosition();
		float distance = sqrt(diff.x*diff.x + diff.y*diff.y + diff.z*diff.z),
		      radius1 = 20.0,
			  radius2;
		
		Ogre::Vector3 scale = node2->getScale();
		if (scale.x == 4.0 && scale.z == 4.0)
			radius2 = 10.0;
		else if (scale.x == 8.0 && scale.z == 2.0)
			radius2 = 5.0;
		else if (scale.x == 3.0 && scale.z == 3.0)
			radius2 = 8.0;
		else if (scale.x == 10.0 && scale.z == 1.0)
			radius2 = 2.0;
		else if (scale.x == 10.0 && scale.z == 10.0)
			radius2 = 20.0;
		else if (scale.x > 0.29 && scale.x < 0.31 && scale.z > 0.29 && scale.z < 0.31)
			radius2 = 20.0;
		else if (scale.x == 5.0 && scale.z == 5.0)
			radius2 = 65.0;
		else if (scale.x == 2.0 && scale.z == 2.0)
			radius2 = 5.0;
		else if (scale.x < 1.9 && scale.z < 1.9)
			radius2 = 3.0;
		else if (scale.x == 7.0 && scale.z == 4.0)
			radius2 = 15.0;
		else if (scale.x == 6.0 && scale.z == 4.0)
			radius2 = 14.0;
		else radius2 = 10.0;
		return distance < radius1 + radius2;
	}

	int collides_any_obstacle(Ogre::SceneNode *node, std::vector<Ogre::SceneNode *> obstacles) {
		for (int i=0; i<obstacles.size(); i++) {
			if (collides(node, obstacles[i])) return i;
		}
		return -1;
	}

	bool collidesCoin(Ogre::SceneNode *node1, Ogre::SceneNode *node2) {
		// determine if there is a sphere to sphere collision
		Ogre::Vector3 diff = node1->_getDerivedPosition() - node2->_getDerivedPosition();
		float distance = sqrt(diff.x*diff.x + diff.y*diff.y + diff.z*diff.z),
		      radius1 = 20.0,
			  radius2 = 10.0;
		Ogre::Vector3 scale = node2->getScale();
		
		return distance < radius1 + radius2;
	}

	int collides_any_coin(Ogre::SceneNode *node) {
		for (int i=0; i<coins.size(); i++) {
			if (collidesCoin(node, coins[i])) return i;
		}
		return -1;
	}

	bool frameStarted(const Ogre::FrameEvent &evt){
		_key->capture();

		float movSpeed=3.0;
		float rotSpeed=1.0;
		Ogre::Vector3 tmov(0,0,0);
		float trot = 0.0;

		if (_key->isKeyDown(OIS::KC_ESCAPE))
			return false;

		if(_key->isKeyDown(OIS::KC_W)) {
			tmov += Ogre::Vector3(0,0,100);
			
			if (!is_accelerating) {
				is_accelerating = true;
				time_accelerating = 0.0;

				for (int i=0; i<4; i++) {
					wheelState[i]->setEnabled(true);
					wheelState[i]->setLoop(false);
					wheelState[i]->setTimePosition(0.0);
				}
			}
		}

		if(_key->isKeyDown(OIS::KC_S)) {
			tmov += Ogre::Vector3(0,0,-100);

			if (!is_accelerating) {
				is_accelerating = true;
				time_accelerating = 0.0;

				for (int i=0; i<4; i++) {
					wheelState[i]->setEnabled(true);
					wheelState[i]->setLoop(false);
					wheelState[i]->setTimePosition(0.0);
				}
			}
		}

		if(_key->isKeyDown(OIS::KC_A)) {
			if (is_accelerating)
				trot += rotSpeed;
		}

		if(_key->isKeyDown(OIS::KC_D)) {
			if (is_accelerating)
				trot -= rotSpeed;
		}
		
		if(is_accelerating) {
			time_accelerating += evt.timeSinceLastFrame;

			if (time_accelerating > 0.5) {
				is_accelerating = false;
				for (int i=0; i<4; i++)
					wheelState[i]->setEnabled(false);
			}
		}

		if(wheelState[0]->getEnabled()) {
			for (int i=0; i<4; i++)
				wheelState[i]->addTime(evt.timeSinceLastFrame);
		}

		// car control
		Ogre::Vector3 initial_position = _node->_getDerivedPosition();
		_node->yaw(Ogre::Degree(trot));
		_node->translate(_node->getOrientation() * tmov * evt.timeSinceLastFrame * movSpeed);

		//Sphere Animation
		animSphere->addTime(evt.timeSinceLastFrame);
		animSphere2->addTime(evt.timeSinceLastFrame);

		for(int i=0; i<9; i++)
			asteroidState[i]->addTime(evt.timeSinceLastFrame);

		// Asteroid Animation
		animAsteroid->addTime(evt.timeSinceLastFrame);
		animAsteroid2->addTime(evt.timeSinceLastFrame);
		animAsteroid3->addTime(evt.timeSinceLastFrame);
		animAsteroid4->addTime(evt.timeSinceLastFrame);
		animAsteroid9->addTime(evt.timeSinceLastFrame);
		animAsteroid10->addTime(evt.timeSinceLastFrame);
		animAsteroid11->addTime(evt.timeSinceLastFrame);
		animAsteroid12->addTime(evt.timeSinceLastFrame);

		int aux = collides_any_obstacle(_node, obstacles);
		if ( aux != -1 || collides_any_wall(_node)) {

			if(aux > 46 && aux < 54){
				_node->_setDerivedPosition(Vector3(0, 0, 6100));
				_node->yaw(Ogre::Degree(-trot));
			} else if(aux > 53 && aux < 62){
				_node->_setDerivedPosition(Vector3(0, 0, 6900));
				_node->yaw(Ogre::Degree(-trot));
			} else if(aux > 60 && aux < 71){
				_node->_setDerivedPosition(Vector3(0, 0, 8150));
				_node->yaw(Ogre::Degree(-trot));	
			} else {
				_node->_setDerivedPosition(initial_position);
				_node->yaw(Ogre::Degree(-trot));
			}
		}

		aux = collides_any_coin(_node);
		if (aux != -1){
			if(!isTakenCoin[aux]){
				_nodeCoins[aux]->setVisible(false, true);
				isTakenCoin[aux] = true;
				// Add to score
			}
		}

		return true;
	}
};


class Example1 : public ExampleApplication
{

public:

	Ogre::FrameListener* FrameListener01;
	Ogre::SceneNode* _nodeChasis01;
	Ogre::Entity* _entChasis01;

	Example1(){
		FrameListener01 = NULL;
	}

	~Example1(){
		if(FrameListener01){
			delete FrameListener01;
		}
	}

	void createFrameListener(){
		FrameListener01 = new FrameListenerClase(_nodeChasis01,_entChasis01,mCamera,mWindow);
		mRoot->addFrameListener(FrameListener01);
	}

	void createCamera() {
		mCamera = mSceneMgr->createCamera("MyCamera1");
		mCamera->setPosition(0,20,-70);
		mCamera->lookAt(0,20,1000);
		mCamera->setNearClipDistance(1);
	}

	void createCoin(String name, String entity, int index, int x, int z){
		_nodeCoins[index] = mSceneMgr->createSceneNode(name);
		mSceneMgr->getRootSceneNode()->addChild(_nodeCoins[index]);
		coins.push_back(_nodeCoins[index]);
		_entCoins[index] = mSceneMgr->createEntity(entity, "cilindro01.mesh");
		_nodeCoins[index]->attachObject(_entCoins[index]);
		_nodeCoins[index]->translate(x,8,z);
		_nodeCoins[index]->setScale(3,0.1,3);
		_nodeCoins[index]->pitch(Degree(90));
		_entCoins[index]->setMaterialName("matArc3");
	}

	void createWall(String name, String entity, int x, int z){
		Ogre::SceneNode* _nodeWall = mSceneMgr->createSceneNode(name);
		mSceneMgr->getRootSceneNode()->addChild(_nodeWall);
		obstacles.push_back(_nodeWall);
		Ogre::Entity* _entWall = mSceneMgr->createEntity(entity, "cubo01.mesh");
		_nodeWall->attachObject(_entWall);
		_nodeWall->yaw(Degree(45));
		_nodeWall->translate(x,5,z);
		_nodeWall->setScale(4,4,4);
	}

	void createWheelAnimation(int wheel_index) {
		// create animation to move wheels
		Real duration = 0.5;
		Real step = duration/2.0;
		Animation* animation = mSceneMgr->createAnimation(wheelName[wheel_index], duration);
		animation->setInterpolationMode(Animation::IM_SPLINE);
		NodeAnimationTrack* track = animation->createNodeTrack(0, _nodeRueda[wheel_index]);

		Ogre::Vector3 wheel_rotation_vector(1.0,0.0,0.0);

		// add keyframes
		TransformKeyFrame* key;
 
		key = track->createNodeKeyFrame(0.0f);
		key->setRotation(Quaternion(Degree(0), wheel_rotation_vector));
		key->setTranslate(_nodeRueda[wheel_index]->getPosition());

		key = track->createNodeKeyFrame(duration);
		key->setRotation(Quaternion(Degree(90), wheel_rotation_vector));
		key->setTranslate(_nodeRueda[wheel_index]->getPosition());

		wheelState[wheel_index] = mSceneMgr->createAnimationState(wheelName[wheel_index]);
	}

	void createAsteroidAnimation(String name, int positionX, Ogre::SceneNode* node, int index) {
		// create animation to move wheels
		Real duration = 4;
		Animation* animation = mSceneMgr->createAnimation(name, duration);
		animation->setInterpolationMode(Animation::IM_SPLINE);
		NodeAnimationTrack* track = animation->createNodeTrack(0, node);

		int y,z,y2,z2, y3, z3;
		y = std::rand()%(50+50)-50;
		z = std::rand()%(10000-8000)+8000;

		y2 = std::rand()%(50+50)-50;
		z2 = std::rand()%(10000-8000)+8000;

		y3 = std::rand()%(50+50)-50;
		z3 = std::rand()%(10000-8000)+8000;

		// add keyframes
		TransformKeyFrame* key;
 
		key = track->createNodeKeyFrame(0.0);
		key->setTranslate(Vector3(positionX, y, z));

		key = track->createNodeKeyFrame(2.0);
		key->setTranslate(Vector3(positionX, y2, z2));

		key = track->createNodeKeyFrame(4.0);
		key->setTranslate(Vector3(positionX, y3, z3));

		asteroidState[index] = mSceneMgr->createAnimationState(name);
		asteroidState[index]->setEnabled(true);
		asteroidState[index]->setLoop(true);
	}

	void createScene()
	{
		mSceneMgr->setAmbientLight(Ogre::ColourValue(0.0, 0.0, 0.0));
		//mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_MODULATIVE);
		
		Ogre::Light* LuzPuntual01 = mSceneMgr->createLight("Luz01");
		LuzPuntual01->setType(Ogre::Light::LT_DIRECTIONAL);
		LuzPuntual01->setDiffuseColour(1.0,1.0,1.0);
		//LuzPuntual01->setDirection(Ogre::Vector3( 0, -1, -1 ));
		LuzPuntual01->setDirection(Ogre::Vector3( 1, -1, -1 ));
		
		Ogre::Light* LuzPuntual03 = mSceneMgr->createLight("Luz03");
		LuzPuntual03->setType(Ogre::Light::LT_DIRECTIONAL);
		LuzPuntual03->setDiffuseColour(1.0,1.0,1.0);
		//LuzPuntual03->setDirection(Ogre::Vector3( 0, -1, 1 ));
		LuzPuntual03->setDirection(Ogre::Vector3( -1, -1, -1 ));

		//Chasis
		_nodeChasis01 = mSceneMgr->createSceneNode("Chasis01");
		_nodeChasis01->attachObject(mCamera);

		// chasis light
		/*Ogre::Light* pointLight = mSceneMgr->createLight("pointLight");
		pointLight->setType(Ogre::Light::LT_POINT);
		pointLight->setDiffuseColour(1.0, 1.0, 1.0);
		pointLight->setSpecularColour(1.0, 1.0, 1.0);
		pointLight->setPosition(Ogre::Vector3(0, 30, -10));
		_nodeChasis01->attachObject(pointLight);*/
		Ogre::Light* spotLight = mSceneMgr->createLight("spotLight");
		spotLight->setType(Ogre::Light::LT_SPOTLIGHT);
		spotLight->setDiffuseColour(1.0, 1.0, 1.0);
		spotLight->setPosition(Ogre::Vector3(0, 20, 0));
		spotLight->setDirection(Ogre::Vector3(0, 50, 1));
		spotLight->setSpotlightRange(Ogre::Degree(10), Ogre::Degree(35));
		_nodeChasis01->attachObject(spotLight);

		mSceneMgr->getRootSceneNode()->addChild(_nodeChasis01);
		
		_entChasis01 = mSceneMgr->createEntity("entChasis01", "chasisCarro.mesh");
		_entChasis01->setMaterialName("shCarro01");
		_nodeChasis01->attachObject(_entChasis01);

		//Ruedas
		//Rueda 1
		_nodeRueda[0] = mSceneMgr->createSceneNode("Rueda01");
		_nodeChasis01->addChild(_nodeRueda[0]);
			
		Ogre::Entity* _entRueda01 = mSceneMgr->createEntity("entRueda01", "ruedaDetallada.mesh");
		_nodeRueda[0]->translate(-5.77,3.517,-9.462);
		_entRueda01->setMaterialName("shRueda02");
		_nodeRueda[0]->attachObject(_entRueda01);

		//Rueda 2
		_nodeRueda[1] = mSceneMgr->createSceneNode("Rueda02");
		_nodeChasis01->addChild(_nodeRueda[1]);
			
		Ogre::Entity* _entRueda02 = mSceneMgr->createEntity("entRueda02", "ruedaDetallada.mesh");
		_nodeRueda[1]->translate(-5.77,3.517,9.462);
		_entRueda02->setMaterialName("shRueda02");
		_nodeRueda[1]->attachObject(_entRueda02);

		//Rueda 3
		_nodeRueda[2] = mSceneMgr->createSceneNode("Rueda03");
		_nodeChasis01->addChild(_nodeRueda[2]);
			
		Ogre::Entity* _entRueda03 = mSceneMgr->createEntity("entRueda03", "ruedaDetallada.mesh");
		_nodeRueda[2]->translate(8.0,3.517,9.462);
		_entRueda03->setMaterialName("shRueda02");
		_nodeRueda[2]->attachObject(_entRueda03);

		//Rueda 4
		_nodeRueda[3] = mSceneMgr->createSceneNode("Rueda04");
		_nodeChasis01->addChild(_nodeRueda[3]);
			
		Ogre::Entity* _entRueda04 = mSceneMgr->createEntity("entRueda04", "ruedaDetallada.mesh");
		_nodeRueda[3]->translate(8.0,3.517,-9.462);
		_entRueda04->setMaterialName("shRueda02");
		_nodeRueda[3]->attachObject(_entRueda04);

		//BordePista
		Ogre::SceneNode* _nodeBPista = mSceneMgr->createSceneNode("BordePista");
		mSceneMgr->getRootSceneNode()->addChild(_nodeBPista);
				
		Ogre::Entity* _entBPista = mSceneMgr->createEntity("BordePista01", "bordePista.mesh");
		_nodeBPista->attachObject(_entBPista);
		
		//PisoObstaculo
		Ogre::SceneNode* _nodePObstaculo = mSceneMgr->createSceneNode("PistaObstaculo");
		mSceneMgr->getRootSceneNode()->addChild(_nodePObstaculo);
				
		Ogre::Entity* _entPObstaculo = mSceneMgr->createEntity("PistaObstaculo", "pisoObstaculo01.mesh");
		_nodePObstaculo->attachObject(_entPObstaculo);

		_entPObstaculo->setMaterialName("matPiso02");
		_entPObstaculo->setCastShadows(true);

		//PisoNOObstaculo
		Ogre::SceneNode* _nodePNObstaculo = mSceneMgr->createSceneNode("PistaNoObstaculo");
		mSceneMgr->getRootSceneNode()->addChild(_nodePNObstaculo);
				
		Ogre::Entity* _entPNOObstaculo = mSceneMgr->createEntity("PistaNoObstaculo", "pisoNoObstaculo01.mesh");
		_nodePNObstaculo->attachObject(_entPNOObstaculo);
		_entPNOObstaculo->setMaterialName("matPiso03");
		_entPNOObstaculo->setCastShadows(true);


		//PosterInicioFinal
		Ogre::SceneNode* _nodePoster = mSceneMgr->createSceneNode("PosterInicioFinal");
		mSceneMgr->getRootSceneNode()->addChild(_nodePoster);
			
		Ogre::Entity* _entPoster = mSceneMgr->createEntity("PosterInicioFinal", "posterInicioFinal.mesh");
		_nodePoster->attachObject(_entPoster);

				
		//BanderaInicial
		Ogre::SceneNode* _nodeBInicial = mSceneMgr->createSceneNode("BanderaInicial");
		mSceneMgr->getRootSceneNode()->addChild(_nodeBInicial);
				
		Ogre::Entity* _entBanderaI = mSceneMgr->createEntity("BanderaInicial", "banderaInicial.mesh");				
		_entBanderaI->setMaterialName("lambert1");				
		_nodeBInicial->attachObject(_entBanderaI);


		//BanderaFinal
		Ogre::SceneNode* _nodeBFinal = mSceneMgr->createSceneNode("BanderaFinal");
		mSceneMgr->getRootSceneNode()->addChild(_nodeBFinal);
		
		Ogre::Entity* _entBanderaF = mSceneMgr->createEntity("BanderaFinal", "banderaFinal.mesh");
		_entBanderaF->setMaterialName("lambert1");
		_nodeBFinal->attachObject(_entBanderaF);

		//Obstaculos 
		Ogre::SceneNode* _nodeObstaculo1 = mSceneMgr->createSceneNode("Obstaculo01");
		obstacles.push_back(_nodeObstaculo1);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo1);
		Ogre::Entity* _entObstaculo = mSceneMgr->createEntity("entObstaculo01", "cubo01.mesh");
		_nodeObstaculo1->attachObject(_entObstaculo);
		_nodeObstaculo1->translate(130,5,460);
		_nodeObstaculo1->setScale(4,4,4);
		
		Ogre::SceneNode* _nodeObstaculo2 = mSceneMgr->createSceneNode("Obstaculo02");
		obstacles.push_back(_nodeObstaculo2);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo2);
		Ogre::Entity* _entObstaculo2 = mSceneMgr->createEntity("entObstaculo02", "cubo02.mesh");
		_nodeObstaculo2->attachObject(_entObstaculo2);
		_nodeObstaculo2->translate(-140,5,460);
		_nodeObstaculo2->setScale(4,4,4);
		
		Ogre::SceneNode* _nodeObstaculo3 = mSceneMgr->createSceneNode("Obstaculo03");
		obstacles.push_back(_nodeObstaculo3);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo3);
		Ogre::Entity* _entObstaculo3 = mSceneMgr->createEntity("entObstaculo03", "cubo01.mesh");
		_nodeObstaculo3->attachObject(_entObstaculo3);
		_nodeObstaculo3->translate(-120,5,530);
		_nodeObstaculo3->yaw(Degree(45));
		_nodeObstaculo3->setScale(4,4,4);

		Ogre::SceneNode* _nodeObstaculo4 = mSceneMgr->createSceneNode("Obstaculo04");
		obstacles.push_back(_nodeObstaculo4);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo4);
		Ogre::Entity* _entObstaculo4 = mSceneMgr->createEntity("entObstaculo04", "cubo01.mesh");
		_nodeObstaculo4->attachObject(_entObstaculo4);
		_nodeObstaculo4->yaw(Degree(30));
		_nodeObstaculo4->translate(10,5,530);
		_nodeObstaculo4->setScale(4,4,4);

		Ogre::SceneNode* _nodeObstaculo5 = mSceneMgr->createSceneNode("Obstaculo05");
		obstacles.push_back(_nodeObstaculo5);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo5);
		Ogre::Entity* _entObstaculo5 = mSceneMgr->createEntity("entObstaculo05", "cubo02.mesh");
		_nodeObstaculo5->attachObject(_entObstaculo5);
		_nodeObstaculo5->translate(-50,5,610);
		_nodeObstaculo5->setScale(4,4,4);

		Ogre::SceneNode* _nodeObstaculo6 = mSceneMgr->createSceneNode("Obstaculo06");
		obstacles.push_back(_nodeObstaculo6);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo6);
		Ogre::Entity* _entObstaculo6 = mSceneMgr->createEntity("entObstaculo06", "cubo02.mesh");
		_nodeObstaculo6->attachObject(_entObstaculo6);
		_nodeObstaculo6->translate(-10,5,710);
		_nodeObstaculo6->setScale(4,4,4);

		Ogre::SceneNode* _nodeObstaculo7 = mSceneMgr->createSceneNode("Obstaculo07");
		obstacles.push_back(_nodeObstaculo7);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo7);
		Ogre::Entity* _entObstaculo7 = mSceneMgr->createEntity("entObstaculo07", "cubo01.mesh");
		_nodeObstaculo7->attachObject(_entObstaculo7);
		_nodeObstaculo7->translate(-130,5,680);
		_nodeObstaculo7->setScale(4,4,4);

		Ogre::SceneNode* _nodeObstaculo8 = mSceneMgr->createSceneNode("Obstaculo08");
		obstacles.push_back(_nodeObstaculo8);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo8);
		Ogre::Entity* _entObstaculo8 = mSceneMgr->createEntity("entObstaculo08", "cubo02.mesh");
		_nodeObstaculo8->attachObject(_entObstaculo8);
		_nodeObstaculo8->translate(-100,5,770);
		_nodeObstaculo8->setScale(4,4,4);

		Ogre::SceneNode* _nodeObstaculo9 = mSceneMgr->createSceneNode("Obstaculo09");
		obstacles.push_back(_nodeObstaculo9);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo9);
		Ogre::Entity* _entObstaculo9 = mSceneMgr->createEntity("entObstaculo09", "cubo02.mesh");
		_nodeObstaculo9->attachObject(_entObstaculo9);
		_nodeObstaculo9->translate(90,5,800);
		_nodeObstaculo9->setScale(4,4,4);

		Ogre::SceneNode* _nodeObstaculo10 = mSceneMgr->createSceneNode("Obstaculo10");
		obstacles.push_back(_nodeObstaculo10);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo10);
		Ogre::Entity* _entObstaculo10 = mSceneMgr->createEntity("entObstaculo10", "cubo02.mesh");
		_nodeObstaculo10->attachObject(_entObstaculo10);
		_nodeObstaculo10->translate(130,5,710);
		_nodeObstaculo10->setScale(4,4,4);

		Ogre::SceneNode* _nodeArco1 = mSceneMgr->createSceneNode("Arco1");
		mSceneMgr->getRootSceneNode()->addChild(_nodeArco1);
		Ogre::Entity* _entArco1 = mSceneMgr->createEntity("entArco1", "torus02.mesh");
		_nodeArco1->attachObject(_entArco1);
		_nodeArco1->translate(0,5,900);
		_nodeArco1->setScale(30,5,30);
		_nodeArco1->pitch(Degree(90));
		_entArco1->setMaterialName("matArc1");

		Ogre::SceneNode* _nodeArco2 = mSceneMgr->createSceneNode("Arco2");
		mSceneMgr->getRootSceneNode()->addChild(_nodeArco2);
		Ogre::Entity* _entArco2 = mSceneMgr->createEntity("entArco2", "torus02.mesh");
		_nodeArco2->attachObject(_entArco2);
		_nodeArco2->translate(0,5,900);
		_nodeArco2->setScale(25,10,25);
		_nodeArco2->pitch(Degree(90));
		_entArco2->setMaterialName("matArc2");

		//Obstaculos 2nd Part

		Ogre::SceneNode* _nodeArco3 = mSceneMgr->createSceneNode("Arco3");
		mSceneMgr->getRootSceneNode()->addChild(_nodeArco3);
		Ogre::Entity* _entArco3 = mSceneMgr->createEntity("entArco3", "torus02.mesh");
		_nodeArco3->attachObject(_entArco3);
		_nodeArco3->translate(0,5,1320);
		_nodeArco3->setScale(30,5,30);
		_nodeArco3->pitch(Degree(90));
		_entArco3->setMaterialName("matArc2");

		Ogre::SceneNode* _nodeArco4 = mSceneMgr->createSceneNode("Arco4");
		mSceneMgr->getRootSceneNode()->addChild(_nodeArco4);
		Ogre::Entity* _entArco4 = mSceneMgr->createEntity("entArco4", "torus02.mesh");
		_nodeArco4->attachObject(_entArco4);
		_nodeArco4->translate(0,5,1320);
		_nodeArco4->setScale(25,10,25);
		_nodeArco4->pitch(Degree(90));
		_entArco4->setMaterialName("matArc1");

		Ogre::SceneNode* _nodeObstaculo11 = mSceneMgr->createSceneNode("Obstaculo11");
		obstacles.push_back(_nodeObstaculo11);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo11);
		Ogre::Entity* _entObstaculo11 = mSceneMgr->createEntity("entObstaculo11", "cubo02.mesh");
		_nodeObstaculo11->attachObject(_entObstaculo11);
		_nodeObstaculo11->translate(50,5,1350);
		_nodeObstaculo11->setScale(4,4,4);

		Ogre::SceneNode* _nodeObstaculo12 = mSceneMgr->createSceneNode("Obstaculo12");
		obstacles.push_back(_nodeObstaculo12);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo12);
		Ogre::Entity* _entObstaculo12 = mSceneMgr->createEntity("entObstaculo12", "cubo02.mesh");
		_nodeObstaculo12->attachObject(_entObstaculo12);
		_nodeObstaculo12->translate(-50,5,1350);
		_nodeObstaculo12->setScale(4,4,4);

		
		Ogre::SceneNode* _nodeObstaculo13 = mSceneMgr->createSceneNode("Obstaculo13");
		obstacles.push_back(_nodeObstaculo13);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo13);
		Ogre::Entity* _entObstaculo13 = mSceneMgr->createEntity("entObstaculo13", "cubo02.mesh");
		_nodeObstaculo13->attachObject(_entObstaculo13);
		_nodeObstaculo13->translate(50,5,1400);
		_nodeObstaculo13->setScale(4,4,4);

		Ogre::SceneNode* _nodeObstaculo14 = mSceneMgr->createSceneNode("Obstaculo14");
		obstacles.push_back(_nodeObstaculo14);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo14);
		Ogre::Entity* _entObstaculo14 = mSceneMgr->createEntity("entObstaculo14", "cubo02.mesh");
		_nodeObstaculo14->attachObject(_entObstaculo14);
		_nodeObstaculo14->translate(-50,5,1400);
		_nodeObstaculo14->setScale(4,4,4);

		Ogre::SceneNode* _nodeObstaculo15 = mSceneMgr->createSceneNode("Obstaculo15");
		obstacles.push_back(_nodeObstaculo15);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo15);
		Ogre::Entity* _entObstaculo15 = mSceneMgr->createEntity("entObstaculo15", "cubo02.mesh");
		_nodeObstaculo15->attachObject(_entObstaculo15);
		_nodeObstaculo15->translate(0,5,1650);
		_nodeObstaculo15->setScale(4,4,4);

		Ogre::SceneNode* _nodeObstaculo16 = mSceneMgr->createSceneNode("Obstaculo16");
		obstacles.push_back(_nodeObstaculo16);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo16);
		Ogre::Entity* _entObstaculo16 = mSceneMgr->createEntity("entObstaculo16", "cubo02.mesh");
		_nodeObstaculo16->attachObject(_entObstaculo16);
		_nodeObstaculo16->translate(35,5,1650);
		_nodeObstaculo16->setScale(4,4,4);

		Ogre::SceneNode* _nodeObstaculo17 = mSceneMgr->createSceneNode("Obstaculo17");
		obstacles.push_back(_nodeObstaculo17);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo17);
		Ogre::Entity* _entObstaculo17 = mSceneMgr->createEntity("entObstaculo17", "cubo02.mesh");
		_nodeObstaculo17->attachObject(_entObstaculo17);
		_nodeObstaculo17->translate(-35,5,1650);
		_nodeObstaculo17->setScale(4,4,4);

		Ogre::SceneNode* _nodeObstaculo18 = mSceneMgr->createSceneNode("Obstaculo18");
		obstacles.push_back(_nodeObstaculo18);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo18);
		Ogre::Entity* _entObstaculo18 = mSceneMgr->createEntity("entObstaculo18", "cubo02.mesh");
		_nodeObstaculo18->attachObject(_entObstaculo18);
		_nodeObstaculo18->translate(70,5,1650);
		_nodeObstaculo18->setScale(4,4,4);

		Ogre::SceneNode* _nodeObstaculo19 = mSceneMgr->createSceneNode("Obstaculo19");
		obstacles.push_back(_nodeObstaculo19);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo19);
		Ogre::Entity* _entObstaculo19 = mSceneMgr->createEntity("entObstaculo19", "cubo02.mesh");
		_nodeObstaculo19->attachObject(_entObstaculo19);
		_nodeObstaculo19->translate(-70,5,1650);
		_nodeObstaculo19->setScale(4,4,4);

		Ogre::SceneNode* _nodeObstaculo20 = mSceneMgr->createSceneNode("Obstaculo20");
		obstacles.push_back(_nodeObstaculo20);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo20);
		Ogre::Entity* _entObstaculo20 = mSceneMgr->createEntity("entObstaculo20", "cubo01.mesh");
		_nodeObstaculo20->attachObject(_entObstaculo20);
		_nodeObstaculo20->translate(55,5,1800);
		_nodeObstaculo20->setScale(8,4,2);
		_nodeObstaculo20->yaw(Degree(-45));

		Ogre::SceneNode* _nodeObstaculo21 = mSceneMgr->createSceneNode("Obstaculo21");
		obstacles.push_back(_nodeObstaculo21);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo21);
		Ogre::Entity* _entObstaculo21 = mSceneMgr->createEntity("entObstaculo21", "cubo01.mesh");
		_nodeObstaculo21->attachObject(_entObstaculo21);
		_nodeObstaculo21->translate(-55,5,1800);
		_nodeObstaculo21->setScale(8,4,2);
		_nodeObstaculo21->yaw(Degree(45));

		Ogre::SceneNode* _nodeObstaculo22 = mSceneMgr->createSceneNode("Obstaculo22");
		obstacles.push_back(_nodeObstaculo22);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo22);
		Ogre::Entity* _entObstaculo22 = mSceneMgr->createEntity("entObstaculo22", "cubo01.mesh");
		_nodeObstaculo22->attachObject(_entObstaculo22);
		_nodeObstaculo22->translate(55,5,2000);
		_nodeObstaculo22->setScale(8,4,2);
		_nodeObstaculo22->yaw(Degree(45));

		Ogre::SceneNode* _nodeObstaculo23 = mSceneMgr->createSceneNode("Obstaculo23");
		obstacles.push_back(_nodeObstaculo23);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo23);
		Ogre::Entity* _entObstaculo23 = mSceneMgr->createEntity("entObstaculo23", "cubo01.mesh");
		_nodeObstaculo23->attachObject(_entObstaculo23);
		_nodeObstaculo23->translate(-55,5,2000);
		_nodeObstaculo23->setScale(8,4,2);
		_nodeObstaculo23->yaw(Degree(-45));

		Ogre::SceneNode* _nodeObstaculo24 = mSceneMgr->createSceneNode("Obstaculo24");
		obstacles.push_back(_nodeObstaculo24);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo24);
		Ogre::Entity* _entObstaculo24 = mSceneMgr->createEntity("entObstaculo24", "cubo01.mesh");
		_nodeObstaculo24->attachObject(_entObstaculo24);
		_nodeObstaculo24->translate(-170,5,1700);
		_nodeObstaculo24->setScale(3,4,3);

		Ogre::SceneNode* _nodeObstaculo25 = mSceneMgr->createSceneNode("Obstaculo25");
		obstacles.push_back(_nodeObstaculo25);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo25);
		Ogre::Entity* _entObstaculo25 = mSceneMgr->createEntity("entObstaculo25", "cubo01.mesh");
		_nodeObstaculo25->attachObject(_entObstaculo25);
		_nodeObstaculo25->translate(170,5,1700);
		_nodeObstaculo25->setScale(3,4,3);

		Ogre::SceneNode* _nodeObstaculo26 = mSceneMgr->createSceneNode("Obstaculo26");
		obstacles.push_back(_nodeObstaculo26);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo26);
		Ogre::Entity* _entObstaculo26 = mSceneMgr->createEntity("entObstaculo26", "cubo02.mesh");
		_nodeObstaculo26->attachObject(_entObstaculo26);
		_nodeObstaculo26->translate(0,5,2200);
		_nodeObstaculo26->setScale(4,4,4);

		Ogre::SceneNode* _nodeObstaculo27 = mSceneMgr->createSceneNode("Obstaculo27");
		obstacles.push_back(_nodeObstaculo27);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo27);
		Ogre::Entity* _entObstaculo27 = mSceneMgr->createEntity("entObstaculo27", "cubo02.mesh");
		_nodeObstaculo27->attachObject(_entObstaculo27);
		_nodeObstaculo27->translate(150,5,2100);
		_nodeObstaculo27->setScale(4,4,4);

		Ogre::SceneNode* _nodeObstaculo28 = mSceneMgr->createSceneNode("Obstaculo28");
		obstacles.push_back(_nodeObstaculo28);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo28);
		Ogre::Entity* _entObstaculo28 = mSceneMgr->createEntity("entObstaculo28", "cubo02.mesh");
		_nodeObstaculo28->attachObject(_entObstaculo28);
		_nodeObstaculo28->translate(-150,5,2100);
		_nodeObstaculo28->setScale(4,4,4);

		Ogre::SceneNode* _nodeObstaculo29 = mSceneMgr->createSceneNode("Obstaculo29");
		obstacles.push_back(_nodeObstaculo29);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo29);
		Ogre::Entity* _entObstaculo29 = mSceneMgr->createEntity("entObstaculo29", "cubo02.mesh");
		_nodeObstaculo29->attachObject(_entObstaculo29);
		_nodeObstaculo29->translate(0,5,1900);
		_nodeObstaculo29->setScale(3,5,3);

		Ogre::SceneNode* _nodeSphere = mSceneMgr->createSceneNode("Sphere");
		obstacles.push_back(_nodeSphere);
		mSceneMgr->getRootSceneNode()->addChild(_nodeSphere);
		Ogre::Entity* _entSphere = mSceneMgr->createEntity("entSphere", "sphere.mesh");
		_nodeSphere->attachObject(_entSphere);
		_nodeSphere->translate(0,20,1900);
		_nodeSphere->setScale(0.1,0.1,0.1);
		_entSphere->setMaterialName("matSphere2");

		Ogre::SceneNode* _nodeSphere2 = mSceneMgr->createSceneNode("Sphere2");
		obstacles.push_back(_nodeSphere2);
		mSceneMgr->getRootSceneNode()->addChild(_nodeSphere2);
		Ogre::Entity* _entSphere2 = mSceneMgr->createEntity("entSphere2", "sphere.mesh");
		_nodeSphere2->attachObject(_entSphere2);
		_nodeSphere2->translate(0,50,1900);
		_nodeSphere2->setScale(0.13,0.13,0.13);
		_entSphere2->setMaterialName("matSphere2");

		Ogre::SceneNode* _nodeArco5 = mSceneMgr->createSceneNode("Arco5");
		mSceneMgr->getRootSceneNode()->addChild(_nodeArco5);
		Ogre::Entity* _entArco5 = mSceneMgr->createEntity("entArco5", "torus02.mesh");
		_nodeArco5->attachObject(_entArco5);
		_nodeArco5->translate(0,5,2350);
		_nodeArco5->setScale(30,5,30);
		_nodeArco5->pitch(Degree(90));
		_entArco5->setMaterialName("matArc3");

		Ogre::SceneNode* _nodeArco6 = mSceneMgr->createSceneNode("Arco6");
		mSceneMgr->getRootSceneNode()->addChild(_nodeArco6);
		Ogre::Entity* _entArco6 = mSceneMgr->createEntity("entArco6", "torus02.mesh");
		_nodeArco6->attachObject(_entArco6);
		_nodeArco6->translate(0,5,2350);
		_nodeArco6->setScale(25,10,25);
		_nodeArco6->pitch(Degree(90));
		_entArco6->setMaterialName("matArc2");

		// Obstaculo 3th Part
		Ogre::SceneNode* _nodeArco7 = mSceneMgr->createSceneNode("Arco7");
		mSceneMgr->getRootSceneNode()->addChild(_nodeArco7);
		Ogre::Entity* _entArco7 = mSceneMgr->createEntity("entArco7", "torus02.mesh");
		_nodeArco7->attachObject(_entArco7);
		_nodeArco7->translate(0,5,5500);
		_nodeArco7->setScale(30,5,30);
		_nodeArco7->pitch(Degree(90));
		_entArco7->setMaterialName("matArc2");

		Ogre::SceneNode* _nodeArco8 = mSceneMgr->createSceneNode("Arco8");
		mSceneMgr->getRootSceneNode()->addChild(_nodeArco8);
		Ogre::Entity* _entArco8 = mSceneMgr->createEntity("entArco8", "torus02.mesh");
		_nodeArco8->attachObject(_entArco8);
		_nodeArco8->translate(0,5,5500);
		_nodeArco8->setScale(25,10,25);
		_nodeArco8->pitch(Degree(90));
		_entArco8->setMaterialName("matArc3");

		Ogre::SceneNode* _nodeObstaculo30 = mSceneMgr->createSceneNode("Obstaculo30");
		obstacles.push_back(_nodeObstaculo30);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo30);
		Ogre::Entity* _entObstaculo30 = mSceneMgr->createEntity("entObstaculo30", "cubo01.mesh");
		_nodeObstaculo30->attachObject(_entObstaculo30);
		_nodeObstaculo30->translate(-150,5,5600);
		_nodeObstaculo30->setScale(8,5,2);
		_nodeObstaculo30->yaw(Degree(45));

		Ogre::SceneNode* _nodeObstaculo31 = mSceneMgr->createSceneNode("Obstaculo31");
		obstacles.push_back(_nodeObstaculo31);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo31);
		Ogre::Entity* _entObstaculo31 = mSceneMgr->createEntity("entObstaculo31", "cubo01.mesh");
		_nodeObstaculo31->attachObject(_entObstaculo31);
		_nodeObstaculo31->translate(150,5,5600);
		_nodeObstaculo31->setScale(8,5,2);
		_nodeObstaculo31->yaw(Degree(-45));

		Ogre::SceneNode* _nodeObstaculo32 = mSceneMgr->createSceneNode("Obstaculo32");
		obstacles.push_back(_nodeObstaculo32);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo32);
		Ogre::Entity* _entObstaculo32 = mSceneMgr->createEntity("entObstaculo32", "cubo01.mesh");
		_nodeObstaculo32->attachObject(_entObstaculo32);
		_nodeObstaculo32->translate(150,5,5700);
		_nodeObstaculo32->setScale(3,5,3);
		_nodeObstaculo32->yaw(Degree(-45));

		Ogre::SceneNode* _nodeObstaculo33 = mSceneMgr->createSceneNode("Obstaculo33");
		obstacles.push_back(_nodeObstaculo33);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo33);
		Ogre::Entity* _entObstaculo33 = mSceneMgr->createEntity("entObstaculo33", "cubo01.mesh");
		_nodeObstaculo33->attachObject(_entObstaculo33);
		_nodeObstaculo33->translate(0,5,5600);
		_nodeObstaculo33->setScale(3,5,3);
		_nodeObstaculo33->yaw(Degree(-45));

		Ogre::SceneNode* _nodeObstaculo34 = mSceneMgr->createSceneNode("Obstaculo34");
		obstacles.push_back(_nodeObstaculo34);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo34);
		Ogre::Entity* _entObstaculo34 = mSceneMgr->createEntity("entObstaculo34", "cubo01.mesh");
		_nodeObstaculo34->attachObject(_entObstaculo34);
		_nodeObstaculo34->translate(-150,5,5700);
		_nodeObstaculo34->setScale(3,5,3);
		_nodeObstaculo34->yaw(Degree(-45));

		Ogre::SceneNode* _nodeObstaculo35 = mSceneMgr->createSceneNode("Obstaculo35");
		obstacles.push_back(_nodeObstaculo35);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo35);
		Ogre::Entity* _entObstaculo35 = mSceneMgr->createEntity("entObstaculo35", "cubo01.mesh");
		_nodeObstaculo35->attachObject(_entObstaculo35);
		_nodeObstaculo35->translate(50,5,5800);
		_nodeObstaculo35->setScale(10,10,1);
		_nodeObstaculo35->yaw(Degree(90));

		Ogre::SceneNode* _nodeObstaculo36 = mSceneMgr->createSceneNode("Obstaculo36");
		obstacles.push_back(_nodeObstaculo36);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo36);
		Ogre::Entity* _entObstaculo36 = mSceneMgr->createEntity("entObstaculo36", "cubo01.mesh");
		_nodeObstaculo36->attachObject(_entObstaculo36);
		_nodeObstaculo36->translate(-50,5,5800);
		_nodeObstaculo36->setScale(10,10,1);
		_nodeObstaculo36->yaw(Degree(90));

		Ogre::SceneNode* _nodeObstaculo37 = mSceneMgr->createSceneNode("Obstaculo37");
		obstacles.push_back(_nodeObstaculo37);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo37);
		Ogre::Entity* _entObstaculo37 = mSceneMgr->createEntity("entObstaculo37", "cubo01.mesh");
		_nodeObstaculo37->attachObject(_entObstaculo37);
		_nodeObstaculo37->translate(120,5,5850);
		_nodeObstaculo37->setScale(10,5,1);
		_nodeObstaculo37->yaw(Degree(90));

		Ogre::SceneNode* _nodeObstaculo38 = mSceneMgr->createSceneNode("Obstaculo38");
		obstacles.push_back(_nodeObstaculo38);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo38);
		Ogre::Entity* _entObstaculo38 = mSceneMgr->createEntity("entObstaculo38", "cubo01.mesh");
		_nodeObstaculo38->attachObject(_entObstaculo38);
		_nodeObstaculo38->translate(-120,5,5850);
		_nodeObstaculo38->setScale(10,5,1);
		_nodeObstaculo38->yaw(Degree(90));

		Ogre::SceneNode* _nodeObstaculo39 = mSceneMgr->createSceneNode("Obstaculo39");
		obstacles.push_back(_nodeObstaculo39);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo39);
		Ogre::Entity* _entObstaculo39 = mSceneMgr->createEntity("entObstaculo39", "cubo01.mesh");
		_nodeObstaculo39->attachObject(_entObstaculo39);
		_nodeObstaculo39->translate(0,50,5800);
		_nodeObstaculo39->setScale(10,0.5,10);
		
		Ogre::SceneNode* _nodeObstaculo40 = mSceneMgr->createSceneNode("Obstaculo40");
		obstacles.push_back(_nodeObstaculo40);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo40);
		Ogre::Entity* _entObstaculo40 = mSceneMgr->createEntity("entObstaculo40", "cubo01.mesh");
		_nodeObstaculo40->attachObject(_entObstaculo40);
		_nodeObstaculo40->translate(0,5,6000);
		_nodeObstaculo40->setScale(4,8,4);

		Ogre::SceneNode* _nodeObstaculo45 = mSceneMgr->createSceneNode("Obstaculo45");
		obstacles.push_back(_nodeObstaculo45);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo45);
		Ogre::Entity* _entObstaculo45 = mSceneMgr->createEntity("entObstaculo45", "cubo01.mesh");
		_nodeObstaculo45->attachObject(_entObstaculo45);
		_nodeObstaculo45->translate(160,5,6000);
		_nodeObstaculo45->setScale(4,8,4);

		Ogre::SceneNode* _nodeObstaculo46 = mSceneMgr->createSceneNode("Obstaculo46");
		obstacles.push_back(_nodeObstaculo46);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo46);
		Ogre::Entity* _entObstaculo46 = mSceneMgr->createEntity("entObstaculo46", "cubo01.mesh");
		_nodeObstaculo46->attachObject(_entObstaculo46);
		_nodeObstaculo46->translate(-160,5,6000);
		_nodeObstaculo46->setScale(4,8,4);

		Ogre::SceneNode* _nodeObstaculo41 = mSceneMgr->createSceneNode("Obstaculo41");
		obstacles.push_back(_nodeObstaculo41);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo41);
		Ogre::Entity* _entObstaculo41 = mSceneMgr->createEntity("entObstaculo41", "cubo02.mesh");
		_nodeObstaculo41->attachObject(_entObstaculo41);
		_nodeObstaculo41->translate(70,5,6200);
		_nodeObstaculo41->setScale(4,8,4);

		Ogre::SceneNode* _nodeObstaculo43 = mSceneMgr->createSceneNode("Obstaculo43");
		obstacles.push_back(_nodeObstaculo43);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo43);
		Ogre::Entity* _entObstaculo43 = mSceneMgr->createEntity("entObstaculo43", "cubo02.mesh");
		_nodeObstaculo43->attachObject(_entObstaculo43);
		_nodeObstaculo43->translate(-70,5,6200);
		_nodeObstaculo43->setScale(4,8,4);

		Ogre::SceneNode* _nodeObstaculo42 = mSceneMgr->createSceneNode("Obstaculo42");
		obstacles.push_back(_nodeObstaculo42);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo42);
		Ogre::Entity* _entObstaculo42 = mSceneMgr->createEntity("entObstaculo42", "cubo02.mesh");
		_nodeObstaculo42->attachObject(_entObstaculo42);
		_nodeObstaculo42->translate(100,5,6200);
		_nodeObstaculo42->setScale(4,8,4);

		Ogre::SceneNode* _nodeObstaculo44 = mSceneMgr->createSceneNode("Obstaculo44");
		obstacles.push_back(_nodeObstaculo44);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo44);
		Ogre::Entity* _entObstaculo44 = mSceneMgr->createEntity("entObstaculo44", "cubo02.mesh");
		_nodeObstaculo44->attachObject(_entObstaculo44);
		_nodeObstaculo44->translate(-100,5,6200);
		_nodeObstaculo44->setScale(4,8,4);

		Ogre::SceneNode* _nodeObstaculo47 = mSceneMgr->createSceneNode("Obstaculo47");
		obstacles.push_back(_nodeObstaculo47);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo47);
		Ogre::Entity* _entObstaculo47 = mSceneMgr->createEntity("entObstaculo47", "sphere.mesh");
		_nodeObstaculo47->attachObject(_entObstaculo47);
		_nodeObstaculo47->translate(-160,5,6300);
		_nodeObstaculo47->setScale(0.3,0.3,0.3);

		// Animation Sphere
		float durationAnim = 4;
		Ogre::Animation* animationSphere01 = mSceneMgr->createAnimation("AnimSphere01",durationAnim);
		animationSphere01->setInterpolationMode(Animation::IM_SPLINE);
		
		Ogre::NodeAnimationTrack* Sphere01track = animationSphere01->createNodeTrack(0, _nodeObstaculo47);
		Ogre::TransformKeyFrame* keySphere;
		
		keySphere = Sphere01track->createNodeKeyFrame(0.0);
		keySphere->setTranslate(Vector3(-160, 5, 6300));
		keySphere->setScale(Vector3(0.3,0.3,0.3));

		keySphere = Sphere01track->createNodeKeyFrame(1.0);
		keySphere->setTranslate(Vector3(0, 5, 6300));
		keySphere->setScale(Vector3(0.3,0.3,0.3));

		keySphere = Sphere01track->createNodeKeyFrame(2.0);
		keySphere->setTranslate(Vector3(160, 5, 6300));
		keySphere->setScale(Vector3(0.3,0.3,0.3));

		keySphere = Sphere01track->createNodeKeyFrame(3.0);
		keySphere->setTranslate(Vector3(0, 5, 6300));
		keySphere->setScale(Vector3(0.3,0.3,0.3));

		keySphere = Sphere01track->createNodeKeyFrame(4.0);
		keySphere->setTranslate(Vector3(-160, 5, 6300));
		keySphere->setScale(Vector3(0.3,0.3,0.3));

		animSphere = mSceneMgr->createAnimationState("AnimSphere01");
		animSphere->setEnabled(true);
		animSphere->setLoop(true);

		Ogre::SceneNode* _nodeObstaculo48 = mSceneMgr->createSceneNode("Obstaculo48");
		obstacles.push_back(_nodeObstaculo48);
		mSceneMgr->getRootSceneNode()->addChild(_nodeObstaculo48);
		Ogre::Entity* _entObstaculo48 = mSceneMgr->createEntity("entObstaculo48", "sphere.mesh");
		_nodeObstaculo48->attachObject(_entObstaculo48);
		_nodeObstaculo48->translate(160,5,6400);
		_nodeObstaculo48->setScale(0.3,0.3,0.3);

		// Animation Sphere 2
		Ogre::Animation* animationSphere02 = mSceneMgr->createAnimation("AnimSphere02",durationAnim);
		animationSphere02->setInterpolationMode(Animation::IM_SPLINE);
		
		Ogre::NodeAnimationTrack* Sphere02track = animationSphere02->createNodeTrack(0, _nodeObstaculo48);
		Ogre::TransformKeyFrame* keySphere2;
		
		keySphere2 = Sphere02track->createNodeKeyFrame(0.0);
		keySphere2->setTranslate(Vector3(160, 5, 6400));
		keySphere2->setScale(Vector3(0.3,0.3,0.3));

		keySphere2 = Sphere02track->createNodeKeyFrame(1.0);
		keySphere2->setTranslate(Vector3(0, 5, 6400));
		keySphere2->setScale(Vector3(0.3,0.3,0.3));

		keySphere2 = Sphere02track->createNodeKeyFrame(2.0);
		keySphere2->setTranslate(Vector3(-160, 5, 6400));
		keySphere2->setScale(Vector3(0.3,0.3,0.3));

		keySphere2 = Sphere02track->createNodeKeyFrame(3.0);
		keySphere2->setTranslate(Vector3(0, 5, 6400));
		keySphere2->setScale(Vector3(0.3,0.3,0.3));

		keySphere2 = Sphere02track->createNodeKeyFrame(4.0);
		keySphere2->setTranslate(Vector3(160, 5, 6400));
		keySphere2->setScale(Vector3(0.3,0.3,0.3));

		animSphere2 = mSceneMgr->createAnimationState("AnimSphere02");
		animSphere2->setEnabled(true);
		animSphere2->setLoop(true);


		// Asteroides
		Ogre::SceneNode* _nodeAsteroid01 = mSceneMgr->createSceneNode("Asteroid01");
		obstacles.push_back(_nodeAsteroid01);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid01);
		Ogre::Entity* _entAsteroid01 = mSceneMgr->createEntity("entAsteroid01", "roca01.mesh");
		_nodeAsteroid01->attachObject(_entAsteroid01);
		_nodeAsteroid01->translate(-160,10,6800);
		_nodeAsteroid01->setScale(5,5,5);

		Ogre::SceneNode* _nodeAsteroid02 = mSceneMgr->createSceneNode("Asteroid02");
		obstacles.push_back(_nodeAsteroid02);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid02);
		Ogre::Entity* _entAsteroid02 = mSceneMgr->createEntity("entAsteroid02", "roca02.mesh");
		_nodeAsteroid02->attachObject(_entAsteroid02);
		_nodeAsteroid02->translate(150,10,6650);
		_nodeAsteroid02->setScale(5,5,5);

		Ogre::SceneNode* _nodeAsteroid03 = mSceneMgr->createSceneNode("Asteroid03");
		obstacles.push_back(_nodeAsteroid03);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid03);
		Ogre::Entity* _entAsteroid03 = mSceneMgr->createEntity("entAsteroid03", "roca03.mesh");
		_nodeAsteroid03->attachObject(_entAsteroid03);
		_nodeAsteroid03->translate(-150,10,6650);
		_nodeAsteroid03->setScale(5,5,5);

		Ogre::SceneNode* _nodeAsteroid04 = mSceneMgr->createSceneNode("Asteroid04");
		obstacles.push_back(_nodeAsteroid04);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid04);
		Ogre::Entity* _entAsteroid04 = mSceneMgr->createEntity("entAsteroid04", "roca04.mesh");
		_nodeAsteroid04->attachObject(_entAsteroid04);
		_nodeAsteroid04->translate(160,10,6900);
		_nodeAsteroid04->setScale(5,5,5);

		Ogre::SceneNode* _nodeAsteroid05 = mSceneMgr->createSceneNode("Asteroid05");
		obstacles.push_back(_nodeAsteroid05);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid05);
		Ogre::Entity* _entAsteroid05 = mSceneMgr->createEntity("entAsteroid05", "roca03.mesh");
		_nodeAsteroid05->attachObject(_entAsteroid05);
		_nodeAsteroid05->translate(160,5,7100);
		_nodeAsteroid05->setScale(2,2,2);

		Ogre::SceneNode* _nodeAsteroid06 = mSceneMgr->createSceneNode("Asteroid06");
		obstacles.push_back(_nodeAsteroid06);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid06);
		Ogre::Entity* _entAsteroid06 = mSceneMgr->createEntity("entAsteroid06", "roca01.mesh");
		_nodeAsteroid06->attachObject(_entAsteroid06);
		_nodeAsteroid06->translate(-160,5,7200);
		_nodeAsteroid06->setScale(2,2,2);

		Ogre::SceneNode* _nodeAsteroid07 = mSceneMgr->createSceneNode("Asteroid07");
		obstacles.push_back(_nodeAsteroid07);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid07);
		Ogre::Entity* _entAsteroid07 = mSceneMgr->createEntity("entAsteroid07", "roca03.mesh");
		_nodeAsteroid07->attachObject(_entAsteroid07);
		_nodeAsteroid07->translate(160,5,7300);
		_nodeAsteroid07->setScale(2,2,2);

		Ogre::SceneNode* _nodeAsteroid08 = mSceneMgr->createSceneNode("Asteroid08");
		obstacles.push_back(_nodeAsteroid08);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid08);
		Ogre::Entity* _entAsteroid08 = mSceneMgr->createEntity("entAsteroid08", "roca02.mesh");
		_nodeAsteroid08->attachObject(_entAsteroid08);
		_nodeAsteroid08->translate(-160,5,7400);
		_nodeAsteroid08->setScale(1.5,1.5,1.5);

		Ogre::SceneNode* _nodeAsteroid09 = mSceneMgr->createSceneNode("Asteroid09");
		obstacles.push_back(_nodeAsteroid09);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid09);
		Ogre::Entity* _entAsteroid09 = mSceneMgr->createEntity("entAsteroid09", "roca03.mesh");
		_nodeAsteroid09->attachObject(_entAsteroid09);
		_nodeAsteroid09->translate(-180,5,7700);
		_nodeAsteroid09->setScale(1,1,1);

		Ogre::SceneNode* _nodeAsteroid10 = mSceneMgr->createSceneNode("Asteroid10");
		obstacles.push_back(_nodeAsteroid10);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid10);
		Ogre::Entity* _entAsteroid10 = mSceneMgr->createEntity("entAsteroid10", "roca03.mesh");
		_nodeAsteroid10->attachObject(_entAsteroid10);
		_nodeAsteroid10->translate(180,5,7700);
		_nodeAsteroid10->setScale(1,1,1);

		Ogre::SceneNode* _nodeAsteroid11 = mSceneMgr->createSceneNode("Asteroid11");
		obstacles.push_back(_nodeAsteroid11);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid11);
		Ogre::Entity* _entAsteroid11 = mSceneMgr->createEntity("entAsteroid11", "roca03.mesh");
		_nodeAsteroid11->attachObject(_entAsteroid11);
		_nodeAsteroid11->translate(90,1000,8000);
		_nodeAsteroid11->setScale(7,4,4);

		Ogre::SceneNode* _nodeAsteroid12 = mSceneMgr->createSceneNode("Asteroid12");
		obstacles.push_back(_nodeAsteroid12);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid12);
		Ogre::Entity* _entAsteroid12 = mSceneMgr->createEntity("entAsteroid12", "roca01.mesh");
		_nodeAsteroid12->attachObject(_entAsteroid12);
		_nodeAsteroid12->translate(-90,1000,8000);
		_nodeAsteroid12->setScale(6,4,4);
		_nodeAsteroid12->yaw(Degree(90));

		// Asteroids Final

		Ogre::SceneNode* _nodeAsteroid13 = mSceneMgr->createSceneNode("Asteroid13");
		obstacles.push_back(_nodeAsteroid13);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid13);
		Ogre::Entity* _entAsteroid13 = mSceneMgr->createEntity("entAsteroid13", "roca01.mesh");
		_nodeAsteroid13->attachObject(_entAsteroid13);
		_nodeAsteroid13->translate(0,8,10000);
		_nodeAsteroid13->setScale(0.5,1,1);
		_nodeAsteroid13->yaw(Degree(90));

		Ogre::SceneNode* _nodeAsteroid14 = mSceneMgr->createSceneNode("Asteroid14");
		obstacles.push_back(_nodeAsteroid14);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid14);
		Ogre::Entity* _entAsteroid14 = mSceneMgr->createEntity("entAsteroid14", "roca02.mesh");
		_nodeAsteroid14->attachObject(_entAsteroid14);
		_nodeAsteroid14->translate(40,8,10000);
		_nodeAsteroid14->setScale(0.5,1,1);
		_nodeAsteroid14->yaw(Degree(90));

		Ogre::SceneNode* _nodeAsteroid15 = mSceneMgr->createSceneNode("Asteroid15");
		obstacles.push_back(_nodeAsteroid15);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid15);
		Ogre::Entity* _entAsteroid15 = mSceneMgr->createEntity("entAsteroid15", "roca02.mesh");
		_nodeAsteroid15->attachObject(_entAsteroid15);
		_nodeAsteroid15->translate(80,8,10000);
		_nodeAsteroid15->setScale(0.5,1,1);

		Ogre::SceneNode* _nodeAsteroid16 = mSceneMgr->createSceneNode("Asteroid16");
		obstacles.push_back(_nodeAsteroid16);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid16);
		Ogre::Entity* _entAsteroid16 = mSceneMgr->createEntity("entAsteroid16", "roca03.mesh");
		_nodeAsteroid16->attachObject(_entAsteroid16);
		_nodeAsteroid16->translate(120,8,10000);
		_nodeAsteroid16->setScale(0.5,1,1);

		Ogre::SceneNode* _nodeAsteroid17 = mSceneMgr->createSceneNode("Asteroid17");
		obstacles.push_back(_nodeAsteroid17);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid17);
		Ogre::Entity* _entAsteroid17 = mSceneMgr->createEntity("entAsteroid17", "roca04.mesh");
		_nodeAsteroid17->attachObject(_entAsteroid17);
		_nodeAsteroid17->translate(160,8,10000);
		_nodeAsteroid17->setScale(0.5,1,1);
		_entAsteroid17->setMaterialName("matAsteroid3");

		Ogre::SceneNode* _nodeAsteroid18 = mSceneMgr->createSceneNode("Asteroid18");
		obstacles.push_back(_nodeAsteroid18);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid18);
		Ogre::Entity* _entAsteroid18 = mSceneMgr->createEntity("entAsteroid18", "roca02.mesh");
		_nodeAsteroid18->attachObject(_entAsteroid18);
		_nodeAsteroid18->translate(-40,8,10000);
		_nodeAsteroid18->setScale(0.5,1,1);
		_nodeAsteroid18->yaw(Degree(70));

		Ogre::SceneNode* _nodeAsteroid19 = mSceneMgr->createSceneNode("Asteroid19");
		obstacles.push_back(_nodeAsteroid19);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid19);
		Ogre::Entity* _entAsteroid19 = mSceneMgr->createEntity("entAsteroid19", "roca01.mesh");
		_nodeAsteroid19->attachObject(_entAsteroid19);
		_nodeAsteroid19->translate(-80,8,10000);
		_nodeAsteroid19->setScale(0.5,1,1);

		Ogre::SceneNode* _nodeAsteroid20 = mSceneMgr->createSceneNode("Asteroid20");
		obstacles.push_back(_nodeAsteroid20);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid20);
		Ogre::Entity* _entAsteroid20 = mSceneMgr->createEntity("entAsteroid20", "roca03.mesh");
		_nodeAsteroid20->attachObject(_entAsteroid20);
		_nodeAsteroid20->translate(-120,8,10000);
		_nodeAsteroid20->setScale(0.5,1,1);

		Ogre::SceneNode* _nodeAsteroid21 = mSceneMgr->createSceneNode("Asteroid21");
		obstacles.push_back(_nodeAsteroid21);
		mSceneMgr->getRootSceneNode()->addChild(_nodeAsteroid21);
		Ogre::Entity* _entAsteroid21 = mSceneMgr->createEntity("entAsteroid21", "roca04.mesh");
		_nodeAsteroid21->attachObject(_entAsteroid21);
		_nodeAsteroid21->translate(-160,8,10000);
		_nodeAsteroid21->setScale(0.5,1,1);
		_entAsteroid21->setMaterialName("matAsteroid3");

		// Material
		_entObstaculo->setMaterialName("matPiso01");
		_entObstaculo2->setMaterialName("matPiso01");
		_entObstaculo3->setMaterialName("matPiso01");
		_entObstaculo4->setMaterialName("matPiso01");
		_entObstaculo5->setMaterialName("matPiso01");
		_entObstaculo6->setMaterialName("matPiso01");
		_entObstaculo7->setMaterialName("matPiso01");
		_entObstaculo8->setMaterialName("matPiso01");
		_entObstaculo9->setMaterialName("matPiso01");
		_entObstaculo10->setMaterialName("matPiso01");
		_entObstaculo11->setMaterialName("matPiso01");
		_entObstaculo12->setMaterialName("matPiso01");
		_entObstaculo13->setMaterialName("matPiso01");
		_entObstaculo14->setMaterialName("matPiso01");
		_entObstaculo15->setMaterialName("matPiso01");
		_entObstaculo16->setMaterialName("matPiso01");
		_entObstaculo17->setMaterialName("matPiso01");
		_entObstaculo18->setMaterialName("matPiso01");
		_entObstaculo19->setMaterialName("matPiso01");
		_entObstaculo20->setMaterialName("matPiso01");
		_entObstaculo21->setMaterialName("matPiso01");
		_entObstaculo22->setMaterialName("matPiso01");
		_entObstaculo23->setMaterialName("matPiso01");
		_entObstaculo24->setMaterialName("matPiso01");
		_entObstaculo25->setMaterialName("matWall");
		_entObstaculo26->setMaterialName("matWall");
		_entObstaculo27->setMaterialName("matWall");
		_entObstaculo28->setMaterialName("matWall");
		_entObstaculo29->setMaterialName("matWall");
		_entObstaculo30->setMaterialName("matWall");
		_entObstaculo31->setMaterialName("matWall");
		_entObstaculo32->setMaterialName("matWall");
		_entObstaculo33->setMaterialName("matWall");
		_entObstaculo34->setMaterialName("matWall");
		_entObstaculo35->setMaterialName("matWall");
		_entObstaculo36->setMaterialName("matWall");
		_entObstaculo37->setMaterialName("matWall");
		_entObstaculo38->setMaterialName("matWall");
		_entObstaculo39->setMaterialName("matWall");
		_entObstaculo40->setMaterialName("matWall");
		_entObstaculo41->setMaterialName("matWall");
		_entObstaculo42->setMaterialName("matWall");
		_entObstaculo43->setMaterialName("matWall");
		_entObstaculo44->setMaterialName("matWall");
		_entObstaculo45->setMaterialName("matWall");
		_entObstaculo46->setMaterialName("matWall");

		_entObstaculo47->setMaterialName("matSphere");
		_entObstaculo48->setMaterialName("matSphere");

		_entAsteroid01->setMaterialName("matAsteroid");
		_entAsteroid02->setMaterialName("matAsteroid2");
		_entAsteroid03->setMaterialName("matAsteroid3");
		_entAsteroid04->setMaterialName("matAsteroid3");
		_entAsteroid05->setMaterialName("matAsteroid2");
		_entAsteroid06->setMaterialName("matAsteroid");
		_entAsteroid07->setMaterialName("matAsteroid2");
		_entAsteroid08->setMaterialName("matAsteroid");
		_entAsteroid09->setMaterialName("matAsteroid2");
		_entAsteroid10->setMaterialName("matAsteroid");
		_entAsteroid11->setMaterialName("matAsteroid3");
		_entAsteroid12->setMaterialName("matAsteroid3");
		_entAsteroid13->setMaterialName("matAsteroid");
		_entAsteroid14->setMaterialName("matAsteroid2");
		_entAsteroid15->setMaterialName("matAsteroid");
		_entAsteroid16->setMaterialName("matAsteroid2");
		_entAsteroid18->setMaterialName("matAsteroid");
		_entAsteroid19->setMaterialName("matAsteroid3");
		_entAsteroid20->setMaterialName("matAsteroid2");

		//Lineal Animated

		float duration = 4;
		Ogre::Animation* animationAsteroid01 = mSceneMgr->createAnimation("AnimAsteroid01",duration);
		animationAsteroid01->setInterpolationMode(Animation::IM_SPLINE);
		
		Ogre::NodeAnimationTrack* Asteroid01track = animationAsteroid01->createNodeTrack(0, _nodeAsteroid05);
		Ogre::TransformKeyFrame* key;
		
		key = Asteroid01track->createNodeKeyFrame(0.0);
		key->setTranslate(Vector3(160, 5, 7100));
		key->setScale(Vector3(2, 2, 2));

		key = Asteroid01track->createNodeKeyFrame(1.0);
		key->setTranslate(Vector3(0, 5, 7100));
		key->setScale(Vector3(2, 2, 2));

		key = Asteroid01track->createNodeKeyFrame(2.0);
		key->setTranslate(Vector3(-160, 5, 7100));
		key->setScale(Vector3(2, 2, 2));

		key = Asteroid01track->createNodeKeyFrame(3.0);
		key->setTranslate(Vector3(0, 5, 7100));
		key->setScale(Vector3(2, 2, 2));

		key = Asteroid01track->createNodeKeyFrame(4.0);
		key->setTranslate(Vector3(160, 5, 7100));
		key->setScale(Vector3(2, 2, 2));

		animAsteroid = mSceneMgr->createAnimationState("AnimAsteroid01");
		animAsteroid->setEnabled(true);
		animAsteroid->setLoop(true);

		Ogre::Animation* animationAsteroid02 = mSceneMgr->createAnimation("AnimAsteroid02",duration);
		animationAsteroid02->setInterpolationMode(Animation::IM_SPLINE);
		
		Ogre::NodeAnimationTrack* Asteroid02track = animationAsteroid02->createNodeTrack(0, _nodeAsteroid06);
		Ogre::TransformKeyFrame* key2;
		
		key2 = Asteroid02track->createNodeKeyFrame(0.0);
		key2->setTranslate(Vector3(-160, 5, 7200));
		key2->setScale(Vector3(2, 2, 2));

		key2 = Asteroid02track->createNodeKeyFrame(1.0);
		key2->setTranslate(Vector3(0, 5, 7200));
		key2->setScale(Vector3(2, 2, 2));

		key2 = Asteroid02track->createNodeKeyFrame(2.0);
		key2->setTranslate(Vector3(160, 5, 7200));
		key2->setScale(Vector3(2, 2, 2));

		key2 = Asteroid02track->createNodeKeyFrame(3.0);
		key2->setTranslate(Vector3(0, 5, 7200));
		key2->setScale(Vector3(2, 2, 2));

		key2 = Asteroid02track->createNodeKeyFrame(4.0);
		key2->setTranslate(Vector3(-160, 5, 7200));
		key2->setScale(Vector3(2, 2, 2));

		animAsteroid2 = mSceneMgr->createAnimationState("AnimAsteroid02");
		animAsteroid2->setEnabled(true);
		animAsteroid2->setLoop(true);
		
		Ogre::Animation* animationAsteroid03 = mSceneMgr->createAnimation("AnimAsteroid03",duration);
		animationAsteroid03->setInterpolationMode(Animation::IM_SPLINE);
		
		Ogre::NodeAnimationTrack* Asteroid03track = animationAsteroid03->createNodeTrack(0, _nodeAsteroid07);
		Ogre::TransformKeyFrame* key3;
		
		key3 = Asteroid03track->createNodeKeyFrame(0.0);
		key3->setTranslate(Vector3(160, 5, 7300));
		key3->setScale(Vector3(2, 2, 2));

		key3 = Asteroid03track->createNodeKeyFrame(1.0);
		key3->setTranslate(Vector3(0, 5, 7300));
		key3->setScale(Vector3(2, 2, 2));

		key3 = Asteroid03track->createNodeKeyFrame(2.0);
		key3->setTranslate(Vector3(-160, 5, 7300));
		key3->setScale(Vector3(2, 2, 2));

		key3 = Asteroid03track->createNodeKeyFrame(3.0);
		key3->setTranslate(Vector3(0, 5, 7300));
		key3->setScale(Vector3(2, 2, 2));

		key3 = Asteroid03track->createNodeKeyFrame(4.0);
		key3->setTranslate(Vector3(160, 5, 7300));
		key3->setScale(Vector3(2, 2, 2));

		animAsteroid3 = mSceneMgr->createAnimationState("AnimAsteroid03");
		animAsteroid3->setEnabled(true);
		animAsteroid3->setLoop(true);

		Ogre::Animation* animationAsteroid04 = mSceneMgr->createAnimation("AnimAsteroid04",duration);
		animationAsteroid04->setInterpolationMode(Animation::IM_SPLINE);
		
		Ogre::NodeAnimationTrack* Asteroid04track = animationAsteroid04->createNodeTrack(0, _nodeAsteroid08);
		Ogre::TransformKeyFrame* key4;
		
		key4 = Asteroid04track->createNodeKeyFrame(0.0);
		key4->setTranslate(Vector3(-160, 5, 7400));
		key4->setScale(Vector3(2, 2, 2));

		key4 = Asteroid04track->createNodeKeyFrame(1.0);
		key4->setTranslate(Vector3(0, 5, 7400));
		key4->setScale(Vector3(2, 2, 2));

		key4 = Asteroid04track->createNodeKeyFrame(2.0);
		key4->setTranslate(Vector3(160, 5, 7400));
		key4->setScale(Vector3(2, 2, 2));

		key4 = Asteroid04track->createNodeKeyFrame(3.0);
		key4->setTranslate(Vector3(0, 5, 7400));
		key4->setScale(Vector3(2, 2, 2));

		key4 = Asteroid04track->createNodeKeyFrame(4.0);
		key4->setTranslate(Vector3(-160, 5, 7400));
		key4->setScale(Vector3(2, 2, 2));

		animAsteroid4 = mSceneMgr->createAnimationState("AnimAsteroid04");
		animAsteroid4->setEnabled(true);
		animAsteroid4->setLoop(true);

		// 9 - 10 Diagonal Animated
		duration = 6;
		Ogre::Animation* animationAsteroid09 = mSceneMgr->createAnimation("AnimAsteroid09",duration);
		animationAsteroid09->setInterpolationMode(Animation::IM_SPLINE);
		
		Ogre::NodeAnimationTrack* Asteroid09track = animationAsteroid09->createNodeTrack(0, _nodeAsteroid09);
		Ogre::TransformKeyFrame* key9;
		
		key9 = Asteroid09track->createNodeKeyFrame(0.0);
		key9->setTranslate(Vector3(-180, 5, 7700));
		key9->setScale(Vector3(2, 2, 2));

		key9 = Asteroid09track->createNodeKeyFrame(2.0);
		key9->setTranslate(Vector3(0, 5, 7550));
		key9->setScale(Vector3(2, 2, 2));

		key9 = Asteroid09track->createNodeKeyFrame(3.0);
		key9->setTranslate(Vector3(180, 5, 7400));
		key9->setScale(Vector3(2, 2, 2));

		key9 = Asteroid09track->createNodeKeyFrame(4.0);
		key9->setTranslate(Vector3(0, 5, 7550));
		key9->setScale(Vector3(2, 2, 2));

		key9 = Asteroid09track->createNodeKeyFrame(6.0);
		key9->setTranslate(Vector3(-180, 5, 7700));
		key9->setScale(Vector3(2, 2, 2));

		animAsteroid9 = mSceneMgr->createAnimationState("AnimAsteroid09");
		animAsteroid9->setEnabled(true);
		animAsteroid9->setLoop(true);

		Ogre::Animation* animationAsteroid10 = mSceneMgr->createAnimation("AnimAsteroid10",duration);
		animationAsteroid10->setInterpolationMode(Animation::IM_SPLINE);
		
		Ogre::NodeAnimationTrack* Asteroid10track = animationAsteroid10->createNodeTrack(0, _nodeAsteroid10);
		Ogre::TransformKeyFrame* key10;
		
		key10 = Asteroid10track->createNodeKeyFrame(0.0);
		key10->setTranslate(Vector3(180, 5, 7700));
		key10->setScale(Vector3(2, 2, 2));

		key10 = Asteroid10track->createNodeKeyFrame(2.0);
		key10->setTranslate(Vector3(0, 5, 7550));
		key10->setScale(Vector3(2, 2, 2));

		key10 = Asteroid10track->createNodeKeyFrame(3.0);
		key10->setTranslate(Vector3(-180, 5, 7400));
		key10->setScale(Vector3(2, 2, 2));

		key10 = Asteroid10track->createNodeKeyFrame(4.0);
		key10->setTranslate(Vector3(0, 5, 7550));
		key10->setScale(Vector3(2, 2, 2));

		key10 = Asteroid10track->createNodeKeyFrame(6.0);
		key10->setTranslate(Vector3(180, 5, 7700));
		key10->setScale(Vector3(2, 2, 2));

		animAsteroid10 = mSceneMgr->createAnimationState("AnimAsteroid10");
		animAsteroid10->setEnabled(true);
		animAsteroid10->setLoop(true);

		// 11 - 12 
		duration = 4;
		Ogre::Animation* animationAsteroid11 = mSceneMgr->createAnimation("AnimAsteroid11",duration);
		animationAsteroid11->setInterpolationMode(Animation::IM_SPLINE);
		
		Ogre::NodeAnimationTrack* Asteroid11track = animationAsteroid11->createNodeTrack(0, _nodeAsteroid11);
		Ogre::TransformKeyFrame* key11;
		
		key11 = Asteroid11track->createNodeKeyFrame(0.0);
		key11->setTranslate(Vector3(90, 1000, 8000));
		key11->setScale(Vector3(7, 4, 4));

		key11 = Asteroid11track->createNodeKeyFrame(1.0);
		key11->setTranslate(Vector3(90, 500, 8000));
		key11->setScale(Vector3(7, 4, 4));

		key11 = Asteroid11track->createNodeKeyFrame(2.0);
		key11->setTranslate(Vector3(90, 0, 8000));
		key11->setScale(Vector3(7, 4, 4));

		key11 = Asteroid11track->createNodeKeyFrame(3.0);
		key11->setTranslate(Vector3(90, -500, 8000));
		key11->setScale(Vector3(7, 4, 4));

		key11 = Asteroid11track->createNodeKeyFrame(4.0);
		key11->setTranslate(Vector3(90, -1000, 8000));
		key11->setScale(Vector3(7, 4, 4));

		animAsteroid11 = mSceneMgr->createAnimationState("AnimAsteroid11");
		animAsteroid11->setEnabled(true);
		animAsteroid11->setLoop(true);

		Ogre::Animation* animationAsteroid12 = mSceneMgr->createAnimation("AnimAsteroid12",duration);
		animationAsteroid12->setInterpolationMode(Animation::IM_SPLINE);
		
		Ogre::NodeAnimationTrack* Asteroid12track = animationAsteroid12->createNodeTrack(0, _nodeAsteroid12);
		Ogre::TransformKeyFrame* key12;
		
		key12 = Asteroid12track->createNodeKeyFrame(0.0);
		key12->setTranslate(Vector3(-90, 1000, 8000));
		key12->setScale(Vector3(6, 4, 4));

		key12 = Asteroid12track->createNodeKeyFrame(1.0);
		key12->setTranslate(Vector3(-90, 500, 8000));
		key12->setScale(Vector3(6, 4, 4));

		key12 = Asteroid12track->createNodeKeyFrame(2.0);
		key12->setTranslate(Vector3(-90, 0, 8000));
		key12->setScale(Vector3(6, 4, 4));

		key12 = Asteroid12track->createNodeKeyFrame(3.0);
		key12->setTranslate(Vector3(-90, -500, 8000));
		key12->setScale(Vector3(6, 4, 4));

		key12 = Asteroid12track->createNodeKeyFrame(4.0);
		key12->setTranslate(Vector3(-90, -1000, 8000));
		key12->setScale(Vector3(6, 4, 4));

		animAsteroid12 = mSceneMgr->createAnimationState("AnimAsteroid12");
		animAsteroid12->setEnabled(true);
		animAsteroid12->setLoop(true);

		createAsteroidAnimation("AnimAsteroid13", 0, _nodeAsteroid13, 0);
		createAsteroidAnimation("AnimAsteroid14", 40, _nodeAsteroid14, 1);
		createAsteroidAnimation("AnimAsteroid15", 80, _nodeAsteroid15, 2);
		createAsteroidAnimation("AnimAsteroid16", 120, _nodeAsteroid16, 3);
		createAsteroidAnimation("AnimAsteroid17", 160, _nodeAsteroid17, 4);
		createAsteroidAnimation("AnimAsteroid18", -40, _nodeAsteroid18, 5);
		createAsteroidAnimation("AnimAsteroid19", -80, _nodeAsteroid19, 6);
		createAsteroidAnimation("AnimAsteroid20", -120, _nodeAsteroid20, 7);
		createAsteroidAnimation("AnimAsteroid21", -160, _nodeAsteroid21, 8);

		createCoin("Coin01", "entCoin01", 0, 0, 100);
		createCoin("Coin02", "entCoin02", 1, 90, 500);
		createCoin("Coin03", "entCoin03", 2, 0, 1000);
		createCoin("Coin04", "entCoin04", 3, 0, 1500);
		createCoin("Coin05", "entCoin05", 4, 0, 2000);
		createCoin("Coin06", "entCoin06", 5, 0, 2500);
		createCoin("Coin07", "entCoin07", 6, 0, 3000);
		createCoin("Coin08", "entCoin08", 7, 0, 3500);
		createCoin("Coin09", "entCoin09", 8, 0, 4000);
		createCoin("Coin10", "entCoin10", 9, 0, 4500);
		createCoin("Coin11", "entCoin11", 10, 0, 5500);
		createCoin("Coin12", "entCoin12", 11, 0, 6500);
		createCoin("Coin13", "entCoin13", 12, 0, 7000);
		createCoin("Coin14", "entCoin14", 13, 0, 8000);
		createCoin("Coin15", "entCoin15", 14, 0, 8500);
		createCoin("Coin16", "entCoin16", 15, 0, 9000);
		createCoin("Coin17", "entCoin17", 16, 100, 9500);
		createCoin("Coin18", "entCoin18", 17, 100, 1500);
		createCoin("Coin19", "entCoin19", 18, -100, 5500);
		createCoin("Coin20", "entCoin20", 19, 140, 1000);

		memset(isTakenCoin, false, 20);

		//SPACE
		mSceneMgr->setSkyDome(true, "matSky", 5, 8);

		// create wheels animations
		for (int i=0; i<4; i++)
			createWheelAnimation(i);
	}

};


int main (void)
{
	Example1 app;
	std::srand(std::time(NULL));
	app.go();
	return 0;
}