#include "BodiesApplication.h"
#include "../types.h"
#include "../n_bodies.h"
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <vector>
 
Application::Application(void)
{
}
 
Application::~Application(void)
{
}

// TODO(laiqu) change this brutal flag to something normal.
bool BRUTAL_ONLINE_SIMULATE_FLAG = true; // indicates if we should playback
                                         // data in a file or simulate it online
 
//-------------------------------------------------------------------------------------
class BodiesFrameListener : public Ogre::FrameListener
{ 
 public:
	BodiesFrameListener(Ogre::SceneManager* mSceneMgr_) : mSceneMgr(mSceneMgr_),
     sim_time(0), lastNode(0), current(0), name_spam(0) {
		std::ifstream in;
        n_bodies::init();
		in.open("out");
		in >> dims >> n;
        while (in.good()) {
            float time_stamp;
            in >> time_stamp;
            bodies.push_back(std::make_pair(time_stamp,
                                      std::vector<n_bodies::Body>()));		
            for (int i = 0; i < n; i++) {
                bodies.back().second.push_back(
                    n_bodies::Body::read_from_stream(in, dims));
            }
            if (BRUTAL_ONLINE_SIMULATE_FLAG) {
                dev_bodies = n_bodies::moveBodiesToDevice(bodies.back().second);
                break;
            }
        }

		in.close();		
	}
	bool frameRenderingQueued(const Ogre::FrameEvent& evt) {
	 
		// Create a SceneNode and attach the Entity to it
		sim_time += evt.timeSinceLastFrame;
		std::cout << sim_time << " " << current << std::endl;
        std::vector<n_bodies::Body> cur_bodies;
        if (BRUTAL_ONLINE_SIMULATE_FLAG) {
            cur_bodies = n_bodies::simulate(dev_bodies, n, evt.timeSinceLastFrame); 
        } else {
            if (sim_time > bodies[current].first && current < bodies.size()) {
                do {
                     current++;
                } while (sim_time > bodies[current].first && current < bodies.size());
                current--;
                cur_bodies = bodies[current].second;
            }
        }
        for (auto ent : entities) {
            delete ent;
        }
        for (auto node : nodes) {
            delete node;
        }
        nodes.clear();
        entities.clear();
        lastNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(std::to_string(name_spam++));
        nodes.push_back(lastNode);
        for (const auto& body : bodies[current].second) {
            Ogre::Entity* e_body = mSceneMgr->createEntity(std::to_string(name_spam++), "sphere.mesh");
            entities.push_back(e_body);
            Ogre::SceneNode* n_body = lastNode->createChildSceneNode(std::to_string(name_spam++));
            nodes.push_back(n_body);
            // Current sphere has 100 radius, so 0.01f makes it radius 1
            float s_r = 0.01f;
            n_body->scale(s_r * body.radius, s_r * body.radius, s_r * body.radius);
            // TODO(laiqu) check if we could simply use Ogre::Vector3. To 
            // tired at the moment.
            std::vector<float> tmp({0, 0, 0});
            for (int j = 0; j < body.position.size(); j++) {
                tmp[j] = body.position[j];
            }	
            n_body->translate(tmp[0], tmp[1], tmp[2]);
            n_body->attachObject(e_body);
        }
		return true;
	}
 protected:
	Ogre::SceneManager* mSceneMgr;
 	Ogre::SceneNode* lastNode;
    std::vector<std::pair<float, std::vector<n_bodies::Body>>> bodies;
    std::vector<Ogre::Entity*> entities;
    std::vector<Ogre::SceneNode*> nodes;
    float sim_time;
 	int current;
    int name_spam;
    int n;
    int dims;
    CUdeviceptr dev_bodies;
};
void Application::createScene(void)
{
    // Set the scene's ambient light
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));
 
    // Create a Light and set its position
    Ogre::Light* light = mSceneMgr->createLight("MainLight");
    light->setPosition(20.0f, 80.0f, 50.0f);
	mRoot->addFrameListener(new BodiesFrameListener(mSceneMgr));	
}

 
 
 
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif
 
#ifdef __cplusplus
extern "C" {
#endif
 
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
        if (argc > 1)
            BRUTAL_ONLINE_SIMULATE_FLAG = false;
        // Create application object
        Application app;
 
        try 
        {
            app.go();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR| MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " << e.getFullDescription().c_str() << std::endl;
#endif
        }
 
        return 0;
    }
 
#ifdef __cplusplus
}
#endif
