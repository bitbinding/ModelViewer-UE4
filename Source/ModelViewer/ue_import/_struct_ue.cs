public struct FVector2D{
    public float X;
    public float Y;
}

public struct FVector{
    public float X;
    public float Y;
    public float Z;
}

public struct FVector4{
    public float X;
    public float Y;
    public float Z;
    public float W;
}

public struct FIntPoint{
    public float X;
    public float Y;
}

public struct FIntVector{
    public float X;
    public float Y;
    public float Z;
}

public struct FRotator{
    float Roll;
    float Pitch;
    float Yaw;
}

public struct FQuat{
    public float X;
    public float Y;
    public float Z;
    public float W;
}



public struct FBox2D{
    public FVector2D Max;
    public FVector2D Min;
}

public struct FBox{
    public FVector Max;
    public FVector Min;
}

public enum ELevelTick{
    LEVELTICK_TimeOnly         = 0,
    LEVELTICK_ViewportsOnly    = 1,
    LEVELTICK_All              = 2,
    LEVELTICK_PauseTick        = 3
}

public struct FColor{
    public int R;
    public int G;
    public int B;
    public int A;
}

public struct FLinearColor{
    public float R;
    public float G;
    public float B;
    public float A;
}

public struct TArray<T> : List<T>
{
    public int ArrayMax;
    public int ArrayNum;
}

public struct FString : string
{
    
}

public struct FMath
{
    
}

public struct FKey
{
    
}
public struct FGeometry
{
}

public struct FMargin
{
}

public struct FRect
{
}
public struct FAnchors
{
}
public struct FRawMesh
{
    
}

public struct FRawMeshBulkData
{
}

public struct FStaticMeshSourceModel
{
}

//commonutils
public struct vectorex<T> : List<T>{

}

public struct StringBuilder
{
}

public struct Vector2
{
}

public struct Vector3
{
}

public struct Vector4
{
}

public struct Bounds
{
}

public struct Rect
{
}


public struct Vector2Int
{
}

public struct Vector3Int
{
}

public struct ByteArray
{
}

public struct Buffer
{
}

public struct TypedArray{}
public struct Int8Array{}
public struct Uint8Array{}
public struct Uint8ClampedArray{}
public struct Int8Array{}
public struct Uint8Array{}
public struct Int16Array{}
public struct Uint16Array{}
public struct Int32Array{}
public struct Uint32Array{}
public struct Float32Array{}
public struct Float64Array{}
public struct Int32Array{}
public struct Uint32Array{}
public struct BigInt64Array{}
public struct BigUint64Array{}

public struct Barrier
{
}

public struct UBarrier
{
}
public struct BarrierOrientation
{
}

public struct UBarrierOrientation
{
}