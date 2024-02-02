#ifndef _MVP_H
#define _MVP_H

#include "glm.h"

struct MVP
{
	glm::mat4 Model;
	glm::mat4 ProjView;
};

#endif
