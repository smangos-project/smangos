using System;
using Godot;

namespace WorldMapStudio.WoW;

/// <summary>
/// Places a model's vertices in the world the way a <c>ModelInstance</c> does: scale, then rotate by the
/// spawn's Euler angles, then translate to its position — all in the vmap frame, where a spawn's
/// horizontal axes are the server's mirrored about the map centre
/// (<c>VMapManager2::convertPositionToInternalRep</c>). The rotation is
/// <c>G3D::Matrix3::fromEulerAnglesZYX</c> over (Y, X, Z) degrees, exactly what the assembler builds its
/// stored bounds with (<c>ModelPosition::init</c>); a spawn's bound is reproduced by it to within a
/// thousandth of a yard, which is what pins the convention.
/// </summary>
public readonly struct WowVmapTransform
{
    /// <summary>Half a map, in yards: the offset between the vmap frame and the server's.</summary>
    public const float MapHalfSize = 32 * 533.3333f;

    private readonly float _m00, _m01, _m02, _m10, _m11, _m12, _m20, _m21, _m22;
    private readonly float _scale;
    private readonly Vector3 _position;

    public WowVmapTransform(Vector3 positionVmap, Vector3 rotationDegrees, float scale)
    {
        _position = positionVmap;
        _scale = scale;

        // Z(yaw) * (Y(pitch) * X(roll)) with yaw = rotation.Y, pitch = rotation.X, roll = rotation.Z.
        float yaw = MathF.PI * rotationDegrees.Y / 180f;
        float pitch = MathF.PI * rotationDegrees.X / 180f;
        float roll = MathF.PI * rotationDegrees.Z / 180f;
        (float sz, float cz) = MathF.SinCos(yaw);
        (float sy, float cy) = MathF.SinCos(pitch);
        (float sx, float cx) = MathF.SinCos(roll);

        // (Y * X)
        float a00 = cy;
        float a01 = sy * sx;
        float a02 = sy * cx;
        float a10 = 0;
        float a11 = cx;
        float a12 = -sx;
        float a20 = -sy;
        float a21 = cy * sx;
        float a22 = cy * cx;

        _m00 = (cz * a00) - (sz * a10);
        _m01 = (cz * a01) - (sz * a11);
        _m02 = (cz * a02) - (sz * a12);
        _m10 = (sz * a00) + (cz * a10);
        _m11 = (sz * a01) + (cz * a11);
        _m12 = (sz * a02) + (cz * a12);
        _m20 = a20;
        _m21 = a21;
        _m22 = a22;
    }

    public WowVmapTransform(WowVmapSpawn spawn) : this(spawn.Position, spawn.Rotation, spawn.Scale)
    {
    }

    /// <summary>A model-space point in the vmap frame.</summary>
    public Vector3 ToVmap(float x, float y, float z)
    {
        x *= _scale;
        y *= _scale;
        z *= _scale;
        return new Vector3(
            (_m00 * x) + (_m01 * y) + (_m02 * z) + _position.X,
            (_m10 * x) + (_m11 * y) + (_m12 * z) + _position.Y,
            (_m20 * x) + (_m21 * y) + (_m22 * z) + _position.Z);
    }

    /// <summary>A model-space point in server-world coordinates.</summary>
    public Vector3 ToServer(float x, float y, float z) => VmapToServer(ToVmap(x, y, z));

    /// <summary>A model-space point in Godot space.</summary>
    public Vector3 ToGodot(float x, float y, float z) => WowCoordinates.ToGodot(ToServer(x, y, z));

    /// <summary>The vmap frame's <c>(x, y, z)</c> as server-world coordinates. Its own inverse.</summary>
    public static Vector3 VmapToServer(Vector3 vmap) => new(MapHalfSize - vmap.X, MapHalfSize - vmap.Y, vmap.Z);
}
