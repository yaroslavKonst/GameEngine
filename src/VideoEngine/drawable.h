#ifndef _DRAWABLE_H
#define _DRAWABLE_H

class Drawable
{
public:
	Drawable();
	virtual ~Drawable();

	virtual bool IsActive()
	{
		return _active && _ready;
	}

	virtual void SetActive(bool active)
	{
		_active = active;
	}

	virtual void SetReady(bool ready)
	{
		_ready = ready;
	}

private:
	bool _active;
	bool _ready;
};

#endif
