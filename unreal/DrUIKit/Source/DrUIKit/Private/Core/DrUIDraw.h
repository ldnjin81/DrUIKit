#pragma once
#include "CoreMinimal.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/CoreStyle.h"

/** 코어 내부 그리기 도구(공개 API 아님). 정점 색으로 그라데이션·면을 칠한다. */
namespace DrUIDraw {
/** 정점 색 삼각형(흰 텍스처). Points는 위젯 로컬 좌표. */
inline void Triangles(FSlateWindowElementList& Out, int32 Layer, const FGeometry& Geometry, const TArray<FVector2f>& Points, const TArray<FLinearColor>& Colors, const TArray<SlateIndex>& Indices) {
  static const FSlateBrush* White = FCoreStyle::Get().GetBrush("GenericWhiteBox");
  const FSlateRenderTransform& Transform = Geometry.GetAccumulatedRenderTransform();
  TArray<FSlateVertex> Verts;
  Verts.Reserve(Points.Num());
  for (int32 I = 0; I < Points.Num(); ++I) Verts.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(Transform, Points[I], FVector2f(.5f), Colors[I].ToFColor(true)));
  FSlateDrawElement::MakeCustomVerts(Out, Layer, FSlateApplication::Get().GetRenderer()->GetResourceHandle(*White), Verts, Indices, nullptr, 0, 0);
}
/** 사각형 그라데이션(From 쪽 변 → To 쪽 변). bVertical이면 위→아래. */
inline void Gradient(FSlateWindowElementList& Out, int32 Layer, const FGeometry& Geometry, FVector2f Min, FVector2f Size, const FLinearColor& From, const FLinearColor& To, bool bVertical) {
  const TArray<FVector2f> P = {Min, Min + FVector2f(Size.X, 0.f), Min + Size, Min + FVector2f(0.f, Size.Y)};
  const TArray<FLinearColor> C = bVertical ? TArray<FLinearColor>{From, From, To, To} : TArray<FLinearColor>{From, To, To, From};
  Triangles(Out, Layer, Geometry, P, C, {0, 1, 2, 0, 2, 3});
}
}
