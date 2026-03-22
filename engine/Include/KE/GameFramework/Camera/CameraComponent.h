#pragma once

#include "KE/GameFramework/Components/TransformComponent.h"
#include "KE/Vulkan/RenderInfo.h"

class CSphereComponent;


class KE_API CCameraComponent : public CTransformComponent
    {
    CHUDDO_DECLARE_CLASS ( CCameraComponent, CTransformComponent );

    public:
        CCameraComponent ( CObject * inOwner = nullptr, const std::string & inDisplayName = "CameraComponent" );
        virtual ~CCameraComponent ();

        virtual void InitComponent () override;
        virtual void Tick ( float DeltaTime ) override;
        virtual void OnBeginPlay () override;
        FMat4 GetViewMatrix () const;
        FMat4 GetProjectionMatrix ( float AspectRatio ) const;
        

        // Для удобства - получить всю информацию о камере одним вызовом
        FCameraInfo GetCameraInfo ( float AspectRatio ) ;
        
        bool IsVisible () const { return bIsVisible; }
        void SetCameraVisible ( bool value ) { bIsVisible = value; }

        void SetFOV ( float value ) { FieldOfView = value; }
        void SetNearClipPlane ( float value ) { NearClipPlane = value; }
        void SetFarClipPlane ( float value ) { FarClipPlane = value; }
        float GetFOV () const { return FieldOfView; }
        float GetNearClipPlane () const { return NearClipPlane; }
        float GetFarClipPlane () const { return FarClipPlane; }
        void UpdateInfo ();

       

    protected:
        bool bIsVisible = true;
        // Camera-specific properties
        float FieldOfView = 90.0f;
        float NearClipPlane = 0.1f;
        float FarClipPlane = 5000.0f;
        FCameraInfo m_CameraInfo;
      
       
      
    };

REGISTER_CLASS_FACTORY ( CCameraComponent );