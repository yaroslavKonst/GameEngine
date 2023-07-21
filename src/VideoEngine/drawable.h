#ifndef _DRAWABLE_H
#define _DRAWABLE_H

class Drawable
{
public:
	Drawable();
	virtual ~Drawable();

	virtual bool IsDrawEnabled()
	{
		return _active && _ready;
	}

	virtual void SetDrawEnabled(bool active)
	{
		_active = active;
	}

	virtual void _SetDrawReady(bool ready)
	{
		_ready = ready;
	}

private:
	bool _active;
	bool _ready;
};

#endif
