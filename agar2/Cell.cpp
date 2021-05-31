#include "Cell.h"
#include "Map.h"
#include <stdint.h>
#include "Color.h"
#include "Vec.h"
#include "Game.h"

uint32_t Cell::CurrentID = 0;
uint32_t Cell::COLORS[Cell::NUM_COLOR]
{
	0xFF0000,
	0x00FF00,
	0x0000FF,
	0x808000,
	0x008080,
	0x800080,
	0xFF8000,
	0x00FF80,
	0x8000FF,

	0xfff607,
	0x3f07ff,
	0x07dbff,
	0x07aaff,
	0x0728ff,
	0x2eff07
};

uint32_t Cell::RandomColor()
{
	return COLORS[rand() % NUM_COLOR];
}

Cell::Cell()
	: Cell(Game::FOOD_RADIUS)
{
}

Cell::Cell(float Radius_, vec2 Position_, uint32_t Color_)
	: Radius(Radius_), _Position(Position_), Color(Color_), ID(CurrentID++)
{
	Init();
}

Cell::~Cell()
{
}

void Cell::Init()
{
}

void Cell::Update(float dt)
{
	// Drag
	// TODO mass multiplier?
	float alpha = IsFood() ? 0 : ((_Force.Dot(_Velocity) + 1) / 2);
	float damping_factor = IsFood() ? Game::FEED_DAMPING : Game::MINIMUM_DAMPING * alpha + Game::MAXIMUM_DAMPING * (1 - alpha);
	float DragMagnitude = _Velocity.Length() * pow(Radius, Game::SIZE_DAMPING) * damping_factor;
	//DragMagnitude = min(100000000, DragMagnitude);

	_Force += _Velocity.Normalize() * DragMagnitude * -1;


	_Acceleration = _Force / Size();
	//_Acceleration *= min(1, 10000 / _Acceleration.Length());
	_Velocity += _Acceleration * dt;
	_MoveRel(_Velocity * dt);
	_Force = 0;
	//if (_Velocity.Length() > 100)
	//{
	//	Velocity *= max(0, 1 - (2 * dt));
	//}
	//else
	//{
	//	//TODO moving too slow, transfer it to static map
	//	_Velocity = vec2(0, 0);
	//}
}

void Cell::_MoveRel(vec2 Offset)
{
	_MoveAbs(_Position + Offset);
}

void Cell::_MoveAbs(vec2 Position)
{
	this->_Position.x = min((float)Map::MAP_SIZE - 0.001, max(0, Position.x));
	this->_Position.y = min((float)Map::MAP_SIZE - 0.001, max(0, Position.y));
}

Cell::Collision Cell::Collide(Cell * cell, float * Overlap, float * PercentOverlap)
{
	//assume for now that one cell is big enough to eat the other
	//
	Collision Result;
	float PercentOverlap_ = 0.f;
	float Distance = (cell->_Position - _Position).Length();
	float SumR = cell->Radius + Radius;
	float SmallD = 2 * min(Radius, cell->Radius);
	float Overlap_ = SumR - Distance;

	if (Overlap)
	{
		*Overlap = Overlap_;
	}
	if (PercentOverlap) {
		*PercentOverlap = PercentOverlap_;
	}
	//if at least touching
	if (Overlap_ >= 0)
	{
		//check if overlapping
		PercentOverlap_ = Overlap_ / SmallD;
		if (PercentOverlap) {
			*PercentOverlap = PercentOverlap_;
		}

		if (PercentOverlap_ > 0.75)
		{
			float SizeDifference = Size() / cell->Size();

			if (SizeDifference > 1.3333)
			{
				return BIGGER;
			}
			else if (SizeDifference < 0.75)
			{
				return SMALLER;
			}
			return OVERLAP;
		}
		return TOUCH;
	}
	return NONE;
}
