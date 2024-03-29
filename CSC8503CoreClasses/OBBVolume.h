#pragma once
#include "CollisionVolume.h"

namespace NCL {
	class OBBVolume : CollisionVolume
	{
	public:
		OBBVolume(const Maths::Vector3& halfDims) {
			type		= VolumeType::OBB;
			halfSizes	= halfDims;
		}
		~OBBVolume() {}

		Maths::Vector3 GetHalfDimensions() const {
			return halfSizes;
		}
		void SetHalfDimensions(const Maths::Vector3& v) {
			halfSizes = v;
		}
	protected:
		Maths::Vector3 halfSizes;
	};
}

