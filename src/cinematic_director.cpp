#include "header/cinematic_director.h"
#include "header/animated_model.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <glm/gtc/matrix_transform.hpp>

CinematicDirector::CinematicDirector(camera_t& cam, glm::mat4& charModel, glm::mat4& cartModel, AnimatedModel* animModel)
    : m_Camera(cam)
    , m_CharacterModel(charModel)
    , m_CartModel(cartModel)
    , m_AnimatedModel(animModel)
    , m_GlobalTime(0.0f)
    , m_IsPlaying(false)
    , m_Loop(false)
    , m_RollStartTime(-1.0f)
    , m_RollFinished(false)
    , m_CollisionTime(-1.0f)
    , m_CollisionPosition(0.0f, 0.0f, 500.0f)
{
    InitializeKeyframes();
    UpdateCharacterMovement(0.0f);
    UpdateCartMovement(0.0f);
    std::cout << "CinematicDirector: Constructor called, " << m_Keyframes.size() << " keyframes initialized" << std::endl;
}

void CinematicDirector::Start() {
    m_GlobalTime = 0.0f;
    m_IsPlaying = true;
    std::cout << "Cinematic Director: Started (Time: " << m_GlobalTime << "s)" << std::endl;
    
    if (m_Keyframes.size() > 0) {
        InterpolateBetweenKeyframes(0.0f);
        std::cout << "  Camera position set to first keyframe" << std::endl;
    } else {
        std::cout << "  WARNING: No keyframes available!" << std::endl;
    }
}

void CinematicDirector::Stop() {
    m_IsPlaying = false;
    std::cout << "Cinematic Director: Stopped" << std::endl;
}

void CinematicDirector::Reset() {
    m_GlobalTime = 0.0f;
    m_IsPlaying = false;
    if (m_Keyframes.size() > 0) {
        UpdateCamera(m_Keyframes[0].cameraPosition, m_Keyframes[0].cameraTarget);
    }
}

void CinematicDirector::Update(float deltaTime) {
    if (!m_IsPlaying) return;
    
    m_GlobalTime += deltaTime;
    
    if (m_Loop && m_Keyframes.size() > 0) {
        float totalDuration = m_Keyframes.back().time + m_Keyframes.back().transitionDuration;
        if (m_GlobalTime >= totalDuration) {
            m_GlobalTime = fmod(m_GlobalTime, totalDuration);
        }
    }
    
    InterpolateBetweenKeyframes(m_GlobalTime);
}

void CinematicDirector::UpdateCameraWithTime(float currentTime) {
    if (!m_IsPlaying) return;
    
    m_GlobalTime = currentTime;
    
    UpdateCharacterMovement(currentTime);
    UpdateCartMovement(currentTime);
    
    if (m_AnimatedModel) {
        UpdateHeadRotation(currentTime);
    }
    
    InterpolateBetweenKeyframes(currentTime);
    
    static float lastCameraOutputTime = -1.0f;
    if (currentTime - lastCameraOutputTime >= 1.0f || lastCameraOutputTime < 0.0f) {
        int timeInt = (int)currentTime;
        std::cout << "Camera Keyframe: Time=" << timeInt 
                  << "s, Position (World)=(" << std::fixed << std::setprecision(4) << m_Camera.position.x << ", " 
                  << m_Camera.position.y << ", " << m_Camera.position.z 
                  << "), LookAt (World)=(" << m_Camera.target.x << ", " << m_Camera.target.y << ", " 
                  << m_Camera.target.z << ")" << std::endl;
        lastCameraOutputTime = currentTime;
    }
}

void CinematicDirector::SeekTo(float time) {
    m_GlobalTime = time;
    InterpolateBetweenKeyframes(m_GlobalTime);
}

void CinematicDirector::InitializeKeyframes() {
    m_Keyframes.clear();
    
    // 0-3s: fixed camera position
    m_Keyframes.push_back(Keyframe(0.0f, 
        glm::vec3(31.3615f, 21.9684f, 120.577f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        3.0f));
    
    // 3-5s: transition to close-up
    m_Keyframes.push_back(Keyframe(3.0f,
        glm::vec3(31.3615f, 21.9684f, 120.577f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        2.0f));
    
    // 5-7s: face close-up, fixed position
    m_Keyframes.push_back(Keyframe(5.0f,
        glm::vec3(-5.0f, 10.3934f, 70.7176f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        2.0f));
    
    // 7-8s: camera fixed, lookat z reversed, following car
    m_Keyframes.push_back(Keyframe(7.0f,
        glm::vec3(-5.0f, 10.3934f, 70.7176f),
        glm::vec3(0.0f, 0.0f, 150.0f),
        1.0f));
    
    // 9-15s: fixed camera position
    m_Keyframes.push_back(Keyframe(9.0f,
        glm::vec3(100.0f, 50.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        6.0f));
    
    // 15-16s: camera transitions to (0, 50, 0)
    m_Keyframes.push_back(Keyframe(15.0f,
        glm::vec3(100.0f, 50.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        1.0f));
    
    // 16-21s: camera fixed at (0, 50, 0), target follows character
    m_Keyframes.push_back(Keyframe(16.0f,
        glm::vec3(0.0f, 50.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        5.0f));
    
    // 21-25s: camera fixed at (0, 50, 0), target follows character
    m_Keyframes.push_back(Keyframe(21.0f,
        glm::vec3(0.0f, 50.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        4.0f));
    
    // 25-30s: camera fixed at (0, 50, 0), target transitions to car
    m_Keyframes.push_back(Keyframe(25.0f,
        glm::vec3(0.0f, 50.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        5.0f));
    
    // 30s: end keyframe
    m_Keyframes.push_back(Keyframe(30.0f,
        glm::vec3(0.0f, 50.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        0.0f));
    
    std::cout << "Cinematic Director: Initialized " << m_Keyframes.size() << " keyframes" << std::endl;
}

void CinematicDirector::InterpolateBetweenKeyframes(float currentTime) {
    if (m_Keyframes.empty()) return;
    
    size_t currentIndex = 0;
    for (size_t i = 0; i < m_Keyframes.size() - 1; i++) {
        if (currentTime >= m_Keyframes[i].time && currentTime < m_Keyframes[i + 1].time) {
            currentIndex = i;
            break;
        }
        if (i == m_Keyframes.size() - 2) {
            currentIndex = i;
        }
    }
    
    // extract character and cart positions from model matrices
    // character model matrix: scale(0.1) * translate(pos) * rotate
    // world position = m_CharacterModel[3].xyz / 0.1
    glm::vec3 characterPos = glm::vec3(m_CharacterModel[3]) / 0.1f;
    
    // cart model matrix: translate(pos) * rotate * scale(10)
    // world position = m_CartModel[3].xyz (no division needed)
    glm::vec3 cartPos = glm::vec3(m_CartModel[3]);
    
    // after last keyframe (30s+), fix camera position and use cart world position for lookat
    if (currentTime >= m_Keyframes.back().time) {
        glm::vec3 actualCartPos = glm::vec3(m_CartModel[3]);
        glm::vec3 cameraPos = glm::vec3(0.0f, 50.0f, 0.0f);
        glm::vec3 cameraTarget = actualCartPos;
        UpdateCamera(cameraPos, cameraTarget);
        return;
    }
    
    const Keyframe& keyframe1 = m_Keyframes[currentIndex];
    const Keyframe& keyframe2 = m_Keyframes[currentIndex + 1];
    
    float timeInSegment = currentTime - keyframe1.time;
    float segmentDuration = keyframe2.time - keyframe1.time;
    float t = (segmentDuration > 0.0f) ? (timeInSegment / segmentDuration) : 0.0f;
    t = glm::clamp(t, 0.0f, 1.0f);
    
    float smoothT = SmoothStep(t);
    
    glm::vec3 cameraPos = glm::mix(keyframe1.cameraPosition, keyframe2.cameraPosition, smoothT);
    glm::vec3 cameraTarget = glm::mix(keyframe1.cameraTarget, keyframe2.cameraTarget, smoothT);
    
    // time-based camera logic (ordered chronologically)
    // 5-7s: camera x follows character x
    if (currentTime >= 5.0f && currentTime < 7.0f) {
        cameraPos.x = characterPos.x - 5.0f;
    }
    // 7-9s: camera position fixed, lookat follows cart
    else if (currentTime >= 7.0f && currentTime < 9.0f) {
        cameraTarget = glm::vec3(cartPos.x, cartPos.y, cartPos.z);
    }
    // 9-15s: fixed camera position and lookat
    else if (currentTime >= 9.0f && currentTime < 15.0f) {
        cameraPos = glm::vec3(100.0f, 50.0f, 0.0f);
        cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    }
    // 15-16s: camera transitions from (100, 50, 0) to (0, 50, 0), lookat follows character
    else if (currentTime >= 15.0f && currentTime < 16.0f) {
        glm::vec3 actualCharacterPos = glm::vec3(m_CharacterModel[3]) / 0.1f;
        float lookAtY = CalculateLookAtY(currentTime);
        cameraTarget = glm::vec3(actualCharacterPos.x, lookAtY, actualCharacterPos.z);
    }
    // 16-21s: camera fixed at (0, 50, 0), lookat follows character
    else if (currentTime >= 16.0f && currentTime < 21.0f) {
        glm::vec3 actualCharacterPos = glm::vec3(m_CharacterModel[3]) / 0.1f;
        cameraPos = glm::vec3(0.0f, 50.0f, 0.0f);
        float lookAtY = CalculateLookAtY(currentTime);
        cameraTarget = glm::vec3(actualCharacterPos.x, lookAtY, actualCharacterPos.z);
        
        static float lastDebugTime = -1.0f;
        if (currentTime - lastDebugTime >= 1.0f || lastDebugTime < 0.0f) {
            glm::vec3 rawModelPos = glm::vec3(m_CharacterModel[3]);
            std::cout << "DEBUG 16-21s (World Coordinates): " << std::endl;
            std::cout << "  Raw Model[3]=(" << rawModelPos.x << ", " << rawModelPos.y << ", " << rawModelPos.z << ")" << std::endl;
            std::cout << "  Character World Pos=(" << actualCharacterPos.x << ", " 
                      << actualCharacterPos.y << ", " << actualCharacterPos.z << ")" << std::endl;
            std::cout << "  Camera World Pos=(" << cameraPos.x << ", " 
                      << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
            std::string lookAtYDesc = (currentTime < 20.0f) ? "50" : 
                                      (currentTime < 21.0f) ? "50->-150 (transitioning)" : "-150";
            std::cout << "  LookAt World Pos (Y = " << lookAtYDesc << ")=(" << cameraTarget.x << ", " 
                      << cameraTarget.y << ", " << cameraTarget.z << ")" << std::endl;
            lastDebugTime = currentTime;
        }
    }
    // 21-27s: camera fixed at (0, 50, 0), lookat follows character with fixed Y
    else if (currentTime >= 21.0f && currentTime < 27.0f) {
        glm::vec3 actualCharacterPos = glm::vec3(m_CharacterModel[3]) / 0.1f;
        cameraPos = glm::vec3(0.0f, 50.0f, 0.0f);
        float lookAtY = -300.0f;
        cameraTarget = glm::vec3(actualCharacterPos.x, lookAtY, actualCharacterPos.z);
    }
    // 27s+: camera fixed at (0, 50, -100), lookat follows cart
    else if (currentTime >= 27.0f) {
        glm::vec3 actualCartPos = glm::vec3(m_CartModel[3]);
        cameraPos = glm::vec3(0.0f, 50.0f, -100.0f);
        cameraTarget = actualCartPos;
    }
    
    UpdateCamera(cameraPos, cameraTarget);
}

float CinematicDirector::CalculateLookAtY(float currentTime) {
    if (currentTime < 20.0f) {
        return 50.0f;
    } else if (currentTime < 21.0f) {
        float transitionProgress = (currentTime - 20.0f) / 1.0f;
        transitionProgress = glm::clamp(transitionProgress, 0.0f, 1.0f);
        float smoothProgress = SmoothStep(transitionProgress);
        return glm::mix(50.0f, -150.0f, smoothProgress);
    } else {
        return -150.0f;
    }
}

void CinematicDirector::UpdateCamera(glm::vec3 position, glm::vec3 target) {
    m_Camera.position = position;
    m_Camera.target = target;
    
    if (glm::length(m_Camera.worldUp) < 0.1f) {
        m_Camera.worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    }
    
    m_Camera.front = glm::normalize(target - position);
    m_Camera.right = glm::normalize(glm::cross(m_Camera.front, m_Camera.worldUp));
    m_Camera.up = glm::normalize(glm::cross(m_Camera.right, m_Camera.front));
    
    glm::vec3 direction = glm::normalize(target - position);
    m_Camera.pitch = glm::degrees(asin(direction.y));
    m_Camera.yaw = glm::degrees(atan2(direction.z, direction.x));
    
    m_Camera.radius = glm::length(target - position);
}

float CinematicDirector::SmoothStep(float t) {
    return t * t * (3.0f - 2.0f * t);
}

void CinematicDirector::PlayWideShot(float t) {
    glm::vec3 pos = glm::vec3(50.0f, 30.0f, 50.0f);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, -25.0f);
    UpdateCamera(pos, target);
}

void CinematicDirector::PlayMediumShot(float t) {
    glm::vec3 pos = glm::vec3(0.0f, 5.0f, 20.0f);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    UpdateCamera(pos, target);
}

void CinematicDirector::PlayCloseUp(float t) {
    glm::vec3 pos = glm::vec3(5.0f, 2.0f, 0.0f);
    glm::vec3 target = glm::vec3(0.0f, 1.0f, 0.0f);
    UpdateCamera(pos, target);
}

void CinematicDirector::PlayFollowShot(float t) {
    glm::vec3 charPos = glm::vec3(m_CharacterModel[3]);
    glm::vec3 offset = glm::vec3(0.0f, 5.0f, 10.0f);
    UpdateCamera(charPos + offset, charPos);
}

void CinematicDirector::PlayOrbitShot(float t) {
    float angle = t * 0.5f;
    float radius = 20.0f;
    glm::vec3 charPos = glm::vec3(m_CharacterModel[3]);
    glm::vec3 pos = charPos + glm::vec3(
        cos(angle) * radius,
        10.0f,
        sin(angle) * radius
    );
    UpdateCamera(pos, charPos);
}

void CinematicDirector::UpdateHeadRotation(float currentTime) {
    if (!m_AnimatedModel) return;
    
    float headRotationStartTime = 5.0f;
    float headRotationEndTime = 6.0f;
    float headRotationDuration = 1.0f;
    
    // bone name lists
    static const std::vector<std::string> headBoneNames = {
        "mixamorig:Head", "Head", "head",
        "mixamorig:Neck", "Neck", "neck",
        "mixamorig:Neck1", "Neck1", "neck1"
    };
    
    static const std::vector<std::string> spineBoneNames = {
        "mixamorig:Spine2", "Spine2", "spine2",
        "mixamorig:Spine1", "Spine1", "spine1",
        "mixamorig:Spine", "Spine", "spine",
        "mixamorig:UpperChest", "UpperChest", "upperChest"
    };
    
    if (currentTime < headRotationStartTime) {
        for (const auto& name : headBoneNames) {
            m_AnimatedModel->clearBoneAdditionalRotation(name);
        }
        for (const auto& name : spineBoneNames) {
            m_AnimatedModel->clearBoneAdditionalRotation(name);
        }
        return;
    }
    
    float rotationProgress = (currentTime - headRotationStartTime) / headRotationDuration;
    
    if (currentTime >= headRotationEndTime) {
        rotationProgress = 1.0f;
    } else {
        rotationProgress = glm::clamp(rotationProgress, 0.0f, 1.0f);
    }
    
    float smoothProgress;
    if (currentTime >= headRotationStartTime && currentTime <= headRotationEndTime) {
        smoothProgress = rotationProgress;
    } else if (currentTime > headRotationEndTime) {
        smoothProgress = 1.0f;
    } else {
        smoothProgress = SmoothStep(rotationProgress);
    }
    
    float maxRotationAngle = 70.0f;
    
    glm::vec3 characterPos = glm::vec3(m_CharacterModel[3]);
    glm::vec3 headPosition = characterPos + glm::vec3(0.0f, 1.5f, 0.0f);
    glm::vec3 toCamera = m_Camera.position - headPosition;
    glm::vec3 cameraDirection = glm::normalize(toCamera);
    
    float horizontalDist = sqrt(cameraDirection.x * cameraDirection.x + cameraDirection.z * cameraDirection.z);
    float targetPitch = glm::degrees(atan2(-cameraDirection.y, horizontalDist));
    
    static float finalTargetPitch = 0.0f;
    static bool finalTargetPitchSet = false;
    
    if (currentTime < headRotationStartTime) {
        finalTargetPitchSet = false;
    }
    
    if (currentTime >= headRotationStartTime && currentTime <= headRotationEndTime) {
        if (m_Camera.position.y < headPosition.y) {
            targetPitch = maxRotationAngle;
        } else {
            targetPitch = -maxRotationAngle;
        }
        if (currentTime >= headRotationEndTime - 0.01f && !finalTargetPitchSet) {
            finalTargetPitch = targetPitch;
            finalTargetPitchSet = true;
        }
    } else if (currentTime > headRotationEndTime) {
        if (finalTargetPitchSet) {
            targetPitch = finalTargetPitch;
        } else {
            targetPitch = (m_Camera.position.y < headPosition.y) ? maxRotationAngle : -maxRotationAngle;
            finalTargetPitch = targetPitch;
            finalTargetPitchSet = true;
        }
    } else {
        if (abs(targetPitch) < 15.0f) {
            if (m_Camera.position.y < headPosition.y) {
                targetPitch = maxRotationAngle;
            } else {
                targetPitch = -maxRotationAngle;
            }
        }
        targetPitch = glm::clamp(targetPitch, -maxRotationAngle, maxRotationAngle);
    }
    
    float rotationAngle = -1 * smoothProgress * targetPitch;
    
    static float lastDebugTime = -1.0f;
    if (currentTime >= headRotationStartTime && currentTime <= headRotationEndTime) {
        if (currentTime - lastDebugTime > 0.2f) {
            lastDebugTime = currentTime;
        }
    }
    
    glm::quat headRotation = glm::angleAxis(glm::radians(rotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    
    bool headRotationApplied = false;
    for (const auto& boneName : headBoneNames) {
        if (m_AnimatedModel->m_BoneInfoMap.find(boneName) != m_AnimatedModel->m_BoneInfoMap.end()) {
            m_AnimatedModel->setBoneAdditionalRotation(boneName, headRotation);
            headRotationApplied = true;
            static float lastHeadPrintTime = -1.0f;
            if (currentTime >= headRotationStartTime && currentTime <= headRotationEndTime) {
                if (currentTime - lastHeadPrintTime > 0.2f) {
                    std::cout << "Head rotation applied to bone: " << boneName << " (angle: " << rotationAngle << " degrees, progress: " << smoothProgress << ")" << std::endl;
                    lastHeadPrintTime = currentTime;
                }
            }
            break;
        }
    }
    
    if (!headRotationApplied && currentTime >= headRotationStartTime && currentTime <= headRotationEndTime) {
        static bool warned = false;
        if (!warned) {
            std::cout << "WARNING: Could not find head bone! Tried: ";
            for (const auto& name : headBoneNames) {
                std::cout << name << " ";
            }
            std::cout << std::endl;
            warned = true;
        }
    }
    
    float spineRotationFactor = 1.0f;
    float spineRotationAngle = rotationAngle * spineRotationFactor;
    glm::quat spineRotation = glm::angleAxis(glm::radians(spineRotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    
    bool spineRotationApplied = false;
    for (const auto& boneName : spineBoneNames) {
        if (m_AnimatedModel->m_BoneInfoMap.find(boneName) != m_AnimatedModel->m_BoneInfoMap.end()) {
            m_AnimatedModel->setBoneAdditionalRotation(boneName, spineRotation);
            spineRotationApplied = true;
            if (currentTime - headRotationStartTime < 0.1f) {
                std::cout << "Spine rotation applied to bone: " << boneName << " (angle: " << spineRotationAngle << " degrees)" << std::endl;
            }
            break;
        }
    }
    
    bool rotationApplied = headRotationApplied || spineRotationApplied;
    
    if (!rotationApplied && currentTime - headRotationStartTime < 0.5f) {
        std::cout << "WARNING: Could not find head/neck bone. Tried: ";
        for (const auto& name : headBoneNames) {
            std::cout << name << " ";
        }
        std::cout << std::endl;
        std::cout << "Available bones: ";
        for (const auto& pair : m_AnimatedModel->m_BoneInfoMap) {
            if (pair.first.find("Head") != std::string::npos || 
                pair.first.find("Neck") != std::string::npos ||
                pair.first.find("head") != std::string::npos ||
                pair.first.find("neck") != std::string::npos) {
                std::cout << pair.first << " ";
            }
        }
        std::cout << std::endl;
    }
}

void CinematicDirector::UpdateCharacterMovement(float currentTime) {
    float walkStartTime = 0.0f;
    float walkDuration = 5.0f;
    float flyEndTime = 20.0f;
    float rollDuration = 5.0f;
    float rotationAngle = -90.0f;
    
    glm::vec3 startPos = glm::vec3(150.0f, 0.0f, 500.0f);
    glm::vec3 walkEndPos = glm::vec3(0.0f, 0.0f, 500.0f);
    float flyDistanceZ = 2400.0f;
    
    if (currentTime < walkStartTime) {
        static bool printed = false;
        if (!printed) {
            std::cout << "Character initial position: x=" << startPos.x << ", y=" << startPos.y << ", z=" << startPos.z << std::endl;
            printed = true;
        }
        BuildCharacterModelMatrix(startPos, rotationAngle);
        return;
    }
    
    glm::vec3 characterPos = glm::vec3(m_CharacterModel[3]) / 0.1f;
    glm::vec3 cartPos = glm::vec3(m_CartModel[3]);
    
    static float lastPositionPrintTime = -1.0f;
    if (currentTime - lastPositionPrintTime >= 1.0f || lastPositionPrintTime < 0.0f) {
        int timeInt = (int)currentTime;
        std::cout << "Time=" << timeInt << "s: Character World Position=(" 
                  << std::fixed << std::setprecision(4) << characterPos.x << ", " 
                  << characterPos.y << ", " << characterPos.z 
                  << "), Cart World Position=(" << cartPos.x << ", " << cartPos.y << ", " << cartPos.z << ")" << std::endl;
        lastPositionPrintTime = currentTime;
    }
    
    float dx = cartPos.x - characterPos.x;
    float dz = cartPos.z - characterPos.z;
    float distance = sqrt(dx * dx + dz * dz);
    
    float xDistance = abs(dx);
    float collisionDistance = 410.0f;
    bool collisionByX = (xDistance < 10.0f) && (distance >= collisionDistance);
    bool collisionByDistance = (distance >= collisionDistance);
    bool distanceCondition = collisionByX || collisionByDistance;
    
    float cartMoveStartTime = 6.0f;
    bool characterReady = (currentTime >= walkStartTime + walkDuration);
    bool cartStarted = (currentTime >= cartMoveStartTime);
    bool notCollidedYet = (m_CollisionTime < 0.0f);
    bool collisionDetected = characterReady && cartStarted && notCollidedYet && distanceCondition;
    
    if (collisionDetected) {
        m_CollisionTime = currentTime;
        m_CollisionPosition = characterPos;
        m_RollStartTime = flyEndTime;
        float flyDuration = flyEndTime - m_CollisionTime;
        std::cout << "COLLISION DETECTED! Distance: " << distance << ", Time: " << currentTime << "s, Character World Position: (" 
                  << m_CollisionPosition.x << ", " << m_CollisionPosition.y << ", " << m_CollisionPosition.z << "), Cart World Position: ("
                  << cartPos.x << ", " << cartPos.y << ", " << cartPos.z << "), Fly Duration: " << flyDuration 
                  << "s, Roll Start Time: " << m_RollStartTime << "s" << std::endl;
    }
    
    if (m_CollisionTime >= 0.0f && currentTime >= m_CollisionTime) {
        float flyDuration = flyEndTime - m_CollisionTime;
        glm::vec3 flyEndPos = glm::vec3(m_CollisionPosition.x, m_CollisionPosition.y, m_CollisionPosition.z - flyDistanceZ);
        
        if (currentTime < flyEndTime) {
            UpdateCharacterFlying(currentTime, flyDuration, flyEndPos);
        } else {
            float rollStartTime = flyEndTime;
            float rollEndTime = rollStartTime + rollDuration;
            
            if (currentTime < rollEndTime) {
                UpdateCharacterRolling(currentTime, rollStartTime, rollDuration, flyEndPos);
            } else {
                if (!m_RollFinished) {
                    m_RollFinished = true;
                    std::cout << "Roll finished! Triggering explode effect..." << std::endl;
                }
                BuildCharacterModelMatrixAfterRoll(flyEndPos);
            }
        }
        return;
    }
    
    if (currentTime > walkStartTime + walkDuration) {
        BuildCharacterModelMatrix(walkEndPos, rotationAngle);
        return;
    }
    
    float walkProgress = (currentTime - walkStartTime) / walkDuration;
    walkProgress = glm::clamp(walkProgress, 0.0f, 1.0f);
    float smoothProgress = SmoothStep(walkProgress);
    
    glm::vec3 currentPos = glm::mix(startPos, walkEndPos, smoothProgress);
    BuildCharacterModelMatrix(currentPos, rotationAngle);
}

void CinematicDirector::UpdateCharacterFlying(float currentTime, float flyDuration, const glm::vec3& flyEndPos) {
    float flyProgress = (currentTime - m_CollisionTime) / flyDuration;
    flyProgress = glm::clamp(flyProgress, 0.0f, 1.0f);
    float smoothFlyProgress = SmoothStep(flyProgress);
    
    float t = smoothFlyProgress;
    float maxHeight = 1080.0f;
    float yPos = 4.0f * maxHeight * t * (1.0f - t);
    
    glm::vec3 currentPos;
    currentPos.x = glm::mix(m_CollisionPosition.x, flyEndPos.x, t);
    currentPos.y = m_CollisionPosition.y + yPos;
    currentPos.z = glm::mix(m_CollisionPosition.z, flyEndPos.z, t);
    
    if (t < 0.5f) {
        float riseFactor = 1.0f + (0.5f - t) * 2.0f;
        yPos *= riseFactor;
        currentPos.y = m_CollisionPosition.y + yPos;
    }
    
    float bodyRotationX = -90.0f;
    float bodyRotationY = -90.0f;
    float additionalYRotation = smoothFlyProgress * -90.0f;
    
    m_CharacterModel = glm::mat4(1.0f);
    m_CharacterModel = glm::scale(m_CharacterModel, glm::vec3(0.1f, 0.1f, 0.1f));
    m_CharacterModel = glm::translate(m_CharacterModel, currentPos);
    m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(additionalYRotation), glm::vec3(0.0f, 1.0f, 0.0f));
    m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(bodyRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
    m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(bodyRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
}

void CinematicDirector::UpdateCharacterRolling(float currentTime, float rollStartTime, float rollDuration, const glm::vec3& flyEndPos) {
    float rollProgress = (currentTime - rollStartTime) / rollDuration;
    rollProgress = glm::clamp(rollProgress, 0.0f, 1.0f);
    float smoothRollProgress = SmoothStep(rollProgress);
    
    float totalRollAngle = -990.0f;
    float currentRollAngle = smoothRollProgress * totalRollAngle;
    
    float flyEndRotationX = -90.0f;
    float flyEndRotationY = -90.0f;
    float additionalYRotation = -90.0f;
    
    m_CharacterModel = glm::mat4(1.0f);
    m_CharacterModel = glm::scale(m_CharacterModel, glm::vec3(0.1f, 0.1f, 0.1f));
    m_CharacterModel = glm::translate(m_CharacterModel, flyEndPos);
    m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(currentRollAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(additionalYRotation), glm::vec3(0.0f, 1.0f, 0.0f));
    m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(flyEndRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
    m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(flyEndRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
}

void CinematicDirector::BuildCharacterModelMatrix(const glm::vec3& position, float rotationAngle) {
    m_CharacterModel = glm::mat4(1.0f);
    m_CharacterModel = glm::scale(m_CharacterModel, glm::vec3(0.1f, 0.1f, 0.1f));
    m_CharacterModel = glm::translate(m_CharacterModel, position);
    m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
}

void CinematicDirector::BuildCharacterModelMatrixAfterRoll(const glm::vec3& position) {
    float rollRotationX = -990.0f;
    float additionalYRotation = -90.0f;
    float flyEndRotationX = -90.0f;
    float flyEndRotationY = -90.0f;
    
    m_CharacterModel = glm::mat4(1.0f);
    m_CharacterModel = glm::scale(m_CharacterModel, glm::vec3(0.1f, 0.1f, 0.1f));
    m_CharacterModel = glm::translate(m_CharacterModel, position);
    m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(rollRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
    m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(additionalYRotation), glm::vec3(0.0f, 1.0f, 0.0f));
    m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(flyEndRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
    m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(flyEndRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
}

void CinematicDirector::UpdateCartMovement(float currentTime) {
    float cartMoveStartTime = 6.0f;
    float cartMoveDuration = 5.0f;
    
    glm::vec3 startPos = glm::vec3(0.0f, 0.0f, 150.0f);
    glm::vec3 endPos = glm::vec3(0.0f, 0.0f, 70.0f);
    float cartRotation = 180.0f;
    glm::vec3 cartScale = glm::vec3(10.0f, 10.0f, 10.0f);
    
    if (currentTime < cartMoveStartTime) {
        BuildCartModelMatrix(startPos, cartRotation, cartScale);
        return;
    }
    
    if (currentTime > cartMoveStartTime + cartMoveDuration) {
        BuildCartModelMatrix(endPos, cartRotation, cartScale);
        return;
    }
    
    float moveProgress = (currentTime - cartMoveStartTime) / cartMoveDuration;
    moveProgress = glm::clamp(moveProgress, 0.0f, 1.0f);
    float smoothProgress = SmoothStep(moveProgress);
    
    glm::vec3 currentPos = glm::mix(startPos, endPos, smoothProgress);
    BuildCartModelMatrix(currentPos, cartRotation, cartScale);
}

void CinematicDirector::BuildCartModelMatrix(const glm::vec3& position, float rotation, const glm::vec3& scale) {
    m_CartModel = glm::mat4(1.0f);
    m_CartModel = glm::translate(m_CartModel, position);
    m_CartModel = glm::rotate(m_CartModel, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    m_CartModel = glm::scale(m_CartModel, scale);
}
