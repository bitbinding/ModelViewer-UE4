#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "PrimitiveSceneProxy.h"
#include <functional>


using DrawGizmosHandler=std::function<void(FPrimitiveDrawInterface* PDI, const FSceneView* view)>;
class FGizmoProxy : public FPrimitiveSceneProxy
{
	DrawGizmosHandler drawGizmosHandler;
public:
	FGizmoProxy(const UPrimitiveComponent* InComponent, DrawGizmosHandler drawGizmosHandler0)
		: FPrimitiveSceneProxy(InComponent)
	{
		//bDrawOnlyIfSelected = true;
		bWillEverBeLit = false;
		drawGizmosHandler = drawGizmosHandler0;
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		

		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = (IsShown(View) && IsSelected());
		Result.bDynamicRelevance = true;
		Result.bShadowRelevance = false;// IsShadowCast(View);
		//Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
		Result.bSeparateTranslucency = Result.bNormalTranslucency = IsShown(View);

		return Result;
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		FMatrix LocalToWorld1 = GetLocalToWorld();

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
				const FSceneView* View = Views[ViewIndex];
				if (drawGizmosHandler != nullptr) {
					drawGizmosHandler(PDI,View);
				}
			}
		}
	}

	virtual uint32 GetMemoryFootprint(void) const override 
	{
		return(sizeof(*this) + GetAllocatedSize()); 
	}

	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}
};
