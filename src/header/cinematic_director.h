#ifndef CINEMATIC_DIRECTOR_H
#define CINEMATIC_DIRECTOR_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include "camera.h"

// keyframe structure
struct Keyframe {
    float time;
    glm::vec3 cameraPosition;
    glm::vec3 cameraTarget;
    float transitionDuration;
    
    Keyframe(float t, glm::vec3 pos, glm::vec3 target, float duration = 1.0f)
        : time(t), cameraPosition(pos), cameraTarget(target), transitionDuration(duration) {}
};

// shot type enumeration
enum class ShotType {
    WIDE_SHOT,
    MEDIUM_SHOT,
    CLOSE_UP,
    FOLLOW,
    ORBIT
};

// forward declaration
class AnimatedModel;

class CinematicDirector {
public:
    CinematicDirector(camera_t& cam, glm::mat4& charModel, glm::mat4& cartModel, AnimatedModel* animModel);
    
    void Start();
    void Stop();
    void Reset();
    void Update(float deltaTime);
    void UpdateCameraWithTime(float currentTime);
    
    bool IsPlaying() const { return m_IsPlaying; }
    float GetCurrentTime() const { return m_GlobalTime; }
    void SetLoop(bool loop) { m_Loop = loop; }
    void SeekTo(float time);
    
    glm::mat4 GetCharacterModelMatrix() const { return m_CharacterModel; }
    glm::mat4 GetCartModelMatrix() const { return m_CartModel; }
    
    void UpdateCharacterMovement(float currentTime);
    void UpdateCartMovement(float currentTime);
    void UpdateHeadRotation(float currentTime);
    
    float GetRollStartTime() const { return m_RollStartTime; }
    bool IsRollFinished() const { return m_RollFinished; }
    
    float GetCollisionTime() const { return m_CollisionTime; }
    glm::vec3 GetCollisionPosition() const { return m_CollisionPosition; }
    bool HasCollisionOccurred() const { return m_CollisionTime >= 0.0f; }

private:
    camera_t& m_Camera;
    glm::mat4& m_CharacterModel;
    glm::mat4& m_CartModel;
    AnimatedModel* m_AnimatedModel;
    
    float m_GlobalTime;
    bool m_IsPlaying;
    bool m_Loop;
    float m_RollStartTime;
    bool m_RollFinished;
    
    float m_CollisionTime;
    glm::vec3 m_CollisionPosition;
    
    std::vector<Keyframe> m_Keyframes;
    
    void InitializeKeyframes();
    void InterpolateBetweenKeyframes(float currentTime);
    void UpdateCamera(glm::vec3 position, glm::vec3 target);
    float SmoothStep(float t);
    
    float CalculateLookAtY(float currentTime);
    
    void UpdateCharacterFlying(float currentTime, float flyDuration, const glm::vec3& flyEndPos);
    void UpdateCharacterRolling(float currentTime, float rollStartTime, float rollDuration, const glm::vec3& flyEndPos);
    void BuildCharacterModelMatrix(const glm::vec3& position, float rotationAngle);
    void BuildCharacterModelMatrixAfterRoll(const glm::vec3& position);
    void BuildCartModelMatrix(const glm::vec3& position, float rotation, const glm::vec3& scale);
    
    void PlayWideShot(float t);
    void PlayMediumShot(float t);
    void PlayCloseUp(float t);
    void PlayFollowShot(float t);
    void PlayOrbitShot(float t);
};

#endif

