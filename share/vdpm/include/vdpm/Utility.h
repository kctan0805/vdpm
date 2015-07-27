#ifndef VDPM_UTILITY_H
#define VDPM_UTILITY_H

#include <cmath>

namespace vdpm
{
	struct Vector
	{
		float x;
		float y;
		float z;

	public:
		Vector() { }

        Vector(float f)
        {
            x = y = z = f;
        }

		Vector(float _x, float _y, float _z)
		{
			x = _x; y = _y; z = _z;
		}

		const float& operator[](int i) const
		{
			return (&x)[i];
		}

		float& operator[](int i)
		{
			return (&x)[i];
		}

		Vector& operator += (const Vector& v)
		{
			x += v.x;   y += v.y;   z += v.z;
			return *this;
		}

        Vector& operator /= (float s)
        {
            x /= s;   y /= s;   z /= s;
            return *this;
        }
	};

	inline Vector operator + (const Vector& v1, const Vector& v2)
	{
		return Vector(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
	}

	inline Vector operator - (const Vector& v1, const Vector& v2)
	{
		return Vector(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
	}

    inline Vector operator * (const Vector& v1, const Vector& v2)
    {
        return Vector(v1.x*v2.x, v1.y*v2.y, v1.z*v2.z);
    }

	inline Vector operator * (const Vector& v, float s)
	{
		return Vector(s*v.x, s*v.y, s*v.z);
	}

	inline Vector operator * (float s, const Vector& v)
	{
		return Vector(s*v.x, s*v.y, s*v.z);
	}

	inline Vector operator / (const Vector& v, float s)
	{
		return Vector(v.x / s, v.y / s, v.z / s);
	}

	inline float dotProduct(const Vector& v1, const Vector& v2)
	{
		return v1.x*v2.x + v1.y * v2.y + v1.z*v2.z;
	}

	extern Vector crossProduct(const Vector& v1, const Vector& v2);

    inline float squareMagnitude(const Vector& v)
    {
        return v.x*v.x + v.y*v.y + v.z*v.z;
    }

    inline float magnitude(const Vector& v)
    {
        return sqrt(squareMagnitude(v));
    }

    struct Bounds
    {
        bool initialized;
        Vector min, max;
        Vector center;
        float radius;
        unsigned int points;

        void addPoint(const Vector& v, bool will_update);
        void complete();
        void merge(const Bounds& b);

        Bounds();
    };

} // namespace vdpm

#endif // VDPM_UTILITY_H
