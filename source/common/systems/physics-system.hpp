#pragma once

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <glm/glm.hpp>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include "../components/collider.hpp"
#include "../components/mesh-renderer.hpp"
#include "../ecs/world.hpp"
#include "../components/instanced-renderer.hpp"
#include <iostream>
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
    std::vector<btTriangleMesh*> triangleMeshes;
    // Character controller for player
    btKinematicCharacterController* characterController = nullptr;
    btPairCachingGhostObject* ghostObject = nullptr;
    btConvexShape* playerShape = nullptr;
    bool playerInitialized = false;


   public:
    void initialize(World *world) {
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

        broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(
            new btGhostPairCallback());

        for (auto entity : world->getEntities())
        {
            auto collisionMesh = entity->getComponent<ColliderComponent>();
            auto meshRenderer = entity->getComponent<MeshRendererComponent>();
            
            if (collisionMesh && meshRenderer && meshRenderer->mesh)
            {
                glm::mat4 transform = entity->getLocalToWorldMatrix();
                addMeshCollision(meshRenderer->mesh, transform, entity);
            }

            auto instancedRenderer = entity->getComponent<InstancedRendererComponent>();
            if (collisionMesh && instancedRenderer && instancedRenderer->mesh)
            {
                for (const auto& instanceMat : instancedRenderer->InstanceMats)
                {
                    addMeshCollision(instancedRenderer->mesh, instanceMat, entity);
                }
            }
        }
        
    }
    void initializePlayerCollider(const glm::vec3 &position, float radius = 0.4f, float height = 1.8f)
    {
        if (playerInitialized)
            return;
        playerShape = new btCapsuleShape(radius, height - 2.0f * radius);

        ghostObject = new btPairCachingGhostObject();
        ghostObject->setCollisionShape(playerShape);
        ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

        btTransform startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(btVector3(position.x, position.y, position.z));
        ghostObject->setWorldTransform(startTransform);

        characterController = new btKinematicCharacterController(
            ghostObject, playerShape, 0.35f, btVector3(0, 1, 0));

        characterController->setGravity(btVector3(0, -30.0f, 0));
        characterController->setMaxJumpHeight(1.5f);
        characterController->setJumpSpeed(8.0f);
        characterController->setFallSpeed(55.0f);
        characterController->setMaxSlope(btRadians(45.0f));

        dynamicsWorld->addCollisionObject(ghostObject,
                                          btBroadphaseProxy::CharacterFilter,
                                          btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
        dynamicsWorld->addAction(characterController);

        playerInitialized = true;
    }
    glm::vec3 getPlayerPosition() const
    {
        if (ghostObject)
        {
            btVector3 pos = ghostObject->getWorldTransform().getOrigin();
            return glm::vec3(pos.x(), pos.y(), pos.z());
        }
        return glm::vec3(0);
    }
    void movePlayer(const glm::vec3 &walkDirection)
    {
        if (characterController)
        {
            characterController->setWalkDirection(
                btVector3(walkDirection.x, walkDirection.y, walkDirection.z));
        }
    }
    bool isPlayerOnGround() const
        {
            return characterController ? characterController->onGround() : false;
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
     void addMeshCollision(Mesh *mesh, const glm::mat4 &transform, void *userPointer = nullptr)
    {
        const auto &Vertices = mesh->getVertices();
        const auto &indices = mesh->getIndices();

        if (Vertices.empty() || indices.empty())
            return;
         
        btTriangleMesh *triangleMesh = new btTriangleMesh();
        triangleMeshes.push_back(triangleMesh);
        // loop through all triangles
        for (size_t i = 0; i < indices.size(); i += 3)
        {
            // Transform vertices to world space
            glm::vec4 v0 = transform * glm::vec4(Vertices[indices[i]].position, 1.0f);
            glm::vec4 v1 = transform * glm::vec4(Vertices[indices[i + 1]].position, 1.0f);
            glm::vec4 v2 = transform * glm::vec4(Vertices[indices[i + 2]].position, 1.0f);

            btVector3 bv0(v0.x, v0.y, v0.z);
            btVector3 bv1(v1.x, v1.y, v1.z);
            btVector3 bv2(v2.x, v2.y, v2.z);

            triangleMesh->addTriangle(bv0, bv1, bv2);
        }
      
        btBvhTriangleMeshShape *meshShape =
            new btBvhTriangleMeshShape(triangleMesh, true);
      

        // Create proper rigid body construction info for static body
        btTransform groundTransform;
        groundTransform.setIdentity();
        
        btDefaultMotionState* motionState = new btDefaultMotionState(groundTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, motionState, meshShape, btVector3(0, 0, 0));
        btRigidBody *body = new btRigidBody(rbInfo);
        
        body->setUserPointer(userPointer);
        body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
        
        
        // Add with proper collision filtering
        dynamicsWorld->addRigidBody(body, btBroadphaseProxy::StaticFilter, 
            btBroadphaseProxy::AllFilter);
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
    bool isPlayerInitialized() const { return playerInitialized; }

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
    // Clean up player controller first
    if (characterController) {
        if (dynamicsWorld) {
            dynamicsWorld->removeAction(characterController);
        }
        delete characterController;
        characterController = nullptr;
    }

    if (ghostObject) {
        if (dynamicsWorld) {
            dynamicsWorld->removeCollisionObject(ghostObject);
        }
        delete ghostObject;
        ghostObject = nullptr;
    }

    if (playerShape) {
        delete playerShape;
        playerShape = nullptr;
    }

    // Reset player initialized flag
    playerInitialized = false;

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
        for (btTriangleMesh *mesh : triangleMeshes)
        {
            delete mesh;
        }
        triangleMeshes.clear();
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
