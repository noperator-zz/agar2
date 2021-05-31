#pragma once
#include "Color.h"
#include "Vec.h"
#include "Map.h"


class Cell
{
private:
	vec2 _Force = vec2(0, 0);
	vec2 _Acceleration = vec2(0, 0);
	vec2 _Velocity = vec2(0, 0);
	vec2 _Position = vec2(0, 0);

public:

	enum Collision
	{
		NONE = 0,
		TOUCH, //between just touching and overlap
		OVERLAP, //overlapping enough to eat one another, but not big enough to eat one another
		BIGGER, //overlapping and big enough to eat
		SMALLER, //overlapping and small enough to be eaten
	};

	union CellFlag
	{
		struct 
		{
			uint8_t IsVirus : 1;
			uint8_t IsFood : 1;
			uint8_t IsPlayer : 1;
			uint8_t Reserved : 5;
		};
		uint8_t u = 0;
	};

	Cell();
	Cell(float Radius_, vec2 Position_ = Map::RandomPosition(), uint32_t Color_ = RandomColor());
	~Cell();

	void IsFood(bool state) { Flag.IsFood = state; }
	bool IsFood() { return Flag.IsFood; }

	void IsVirus(bool state) { Flag.IsVirus = state; }
	bool IsVirus() { return Flag.IsVirus; }

	void _MoveRel(vec2 Offset);
	void _MoveAbs(vec2 Position);
	inline vec2 Center() { return _Position;  }
	Collision Collide(Cell * cell, float * Overlap = nullptr, float * PercentOverlap = nullptr);
	inline float Size() { return Radius * Radius * 0.01; }
	inline void AddSize(float size) { size += Size(); Radius = sqrt(size * 100.f); }
	void Update(float dt);

	//float GetRadius() { return Radius; }
	inline vec2 GetForce() { return _Force; }
	inline vec2 AddForce(vec2 Force) { _Force += Force; return _Force; }
	inline vec2 _AddVelocity(vec2 Velocity) { _Velocity += Velocity; return _Velocity; }
	inline vec2 GetAcceleration() { return _Acceleration; }
	inline vec2 GetVelocity() { return _Velocity; }

	static constexpr inline float GetSize(const float radius) { return radius * radius * 0.01f; }
	static inline float GetRadius(const float size) { return sqrt(size * 100.f); }

	uint32_t ID;
	uint32_t Color;
	CellFlag Flag;
	float Radius;
	vec2 MouseVector;
	float CombineTime;


protected:

private:

	static constexpr float EAT_MULT = 0.75;

	static constexpr uint32_t NUM_COLOR = 15;
	static uint32_t COLORS[NUM_COLOR];

	static uint32_t CurrentID;

	static uint32_t RandomColor();
	void UpdateView();
	void Init();

};

