#pragma once

#include <btBulletDynamicsCommon.h>

#include <glm/glm.hpp>
#include <vector>

namespace our {

// Raycast result structure
struct RaycastResult {
    bool hit = false;
    glm::vec3 hitPoint = glm::vec3(0.0f);
    glm::vec3 hitNormal = glm::vec3(0.0f);
    float hitFraction = 1.0f;
    void* userData = nullptr;  // Can store Entity* pointer
};

class PhysicsSystem {
   private:
    btDefaultCollisionConfiguration* collisionConfig = nullptr;
    btCollisionDispatcher* dispatcher = nullptr;
    btBroadphaseInterface* broadphase = nullptr;
    btSequentialImpulseConstraintSolver* solver = nullptr;
    btDiscreteDynamicsWorld* dynamicsWorld = nullptr;

   public:
    void initialize() {
        // Collision configuration
        collisionConfig = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher(collisionConfig);

        // Broadphase - AABB tree for efficient collision detection
        broadphase = new btDbvtBroadphase();

        // Constraint solver
        solver = new btSequentialImpulseConstraintSolver();

        // The dynamics world
        dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase,
                                                    solver, collisionConfig);

        // Set gravity (optional, not needed for pure raycasting)
        dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
    }

    void update(float deltaTime) {
        if (dynamicsWorld) {
            dynamicsWorld->stepSimulation(deltaTime, 10);
        }
    }

    // Simple raycast - returns first hit
    RaycastResult raycast(const glm::vec3& from, const glm::vec3& to) {
        RaycastResult result;

        if (!dynamicsWorld) return result;

        btVector3 btFrom(from.x, from.y, from.z);
        btVector3 btTo(to.x, to.y, to.z);

        btCollisionWorld::ClosestRayResultCallback rayCallback(btFrom, btTo);
        dynamicsWorld->rayTest(btFrom, btTo, rayCallback);

        if (rayCallback.hasHit()) {
            result.hit = true;
            result.hitPoint = glm::vec3(rayCallback.m_hitPointWorld.x(),
                                        rayCallback.m_hitPointWorld.y(),
                                        rayCallback.m_hitPointWorld.z());
            result.hitNormal = glm::vec3(rayCallback.m_hitNormalWorld.x(),
                                         rayCallback.m_hitNormalWorld.y(),
                                         rayCallback.m_hitNormalWorld.z());
            result.hitFraction = rayCallback.m_closestHitFraction;
            result.userData = rayCallback.m_collisionObject->getUserPointer();
        }

        return result;
    }

    // Raycast with max distance
    RaycastResult raycast(const glm::vec3& origin, const glm::vec3& direction,
                          float maxDistance) {
        glm::vec3 to = origin + glm::normalize(direction) * maxDistance;
        return raycast(origin, to);
    }

    // Add a static collision box (for pages, walls, etc.)
    btRigidBody* addStaticBox(const glm::vec3& position,
                              const glm::vec3& halfExtents,
                              void* userPointer = nullptr) {
        btCollisionShape* shape = new btBoxShape(
            btVector3(halfExtents.x, halfExtents.y, halfExtents.z));

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(position.x, position.y, position.z));

        btDefaultMotionState* motionState = new btDefaultMotionState(transform);

        // Mass = 0 means static object
        btRigidBody::btRigidBodyConstructionInfo rbInfo(0, motionState, shape);
        btRigidBody* body = new btRigidBody(rbInfo);

        body->setUserPointer(userPointer);
        dynamicsWorld->addRigidBody(body);

        return body;
    }

    // Add a static sphere collider
    btRigidBody* addStaticSphere(const glm::vec3& position, float radius,
                                 void* userPointer = nullptr) {
        btCollisionShape* shape = new btSphereShape(radius);

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(position.x, position.y, position.z));

        btDefaultMotionState* motionState = new btDefaultMotionState(transform);

        btRigidBody::btRigidBodyConstructionInfo rbInfo(0, motionState, shape);
        btRigidBody* body = new btRigidBody(rbInfo);

        body->setUserPointer(userPointer);
        dynamicsWorld->addRigidBody(body);

        return body;
    }

    // Remove a rigid body
    void removeBody(btRigidBody* body) {
        if (dynamicsWorld && body) {
            dynamicsWorld->removeRigidBody(body);
            delete body->getMotionState();
            delete body->getCollisionShape();
            delete body;
        }
    }

    btDiscreteDynamicsWorld* getWorld() { return dynamicsWorld; }

    void destroy() {
        if (dynamicsWorld) {
            // Remove all rigid bodies
            for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0;
                 i--) {
                btCollisionObject* obj =
                    dynamicsWorld->getCollisionObjectArray()[i];
                btRigidBody* body = btRigidBody::upcast(obj);
                if (body && body->getMotionState()) {
                    delete body->getMotionState();
                }
                dynamicsWorld->removeCollisionObject(obj);
                delete obj->getCollisionShape();
                delete obj;
            }

            delete dynamicsWorld;
            delete solver;
            delete broadphase;
            delete dispatcher;
            delete collisionConfig;

            dynamicsWorld = nullptr;
            solver = nullptr;
            broadphase = nullptr;
            dispatcher = nullptr;
            collisionConfig = nullptr;
        }
    }

    ~PhysicsSystem() { destroy(); }
};

}  // namespace our
