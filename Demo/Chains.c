/* Copyright (c) 2007 Scott Lembcke
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
#include <stdlib.h>
#include <math.h>

#include "chipmunk.h"
#include "ChipmunkDemo.h"

static cpSpace *space;

#define CHAIN_COUNT 8
#define LINK_COUNT 10
#define JOINT_COUNT (CHAIN_COUNT*LINK_COUNT)
cpConstraint *breakableJoints[JOINT_COUNT] = {};

static void
update(int ticks)
{
	int steps = 3;
	cpFloat dt = 1.0f/60.0f/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
		
		for(int i=0; i<JOINT_COUNT; i++){
			cpConstraint *joint = breakableJoints[i];
			if(joint == NULL) continue;
			
			// Convert the impulse to a force by dividing it by the timestep.
			cpFloat force = cpConstraintGetImpulse(joint)/dt;
			cpFloat maxForce = cpConstraintGetMaxForce(joint);

			// If the force is almost as big as the joint's max force, break it.
			if(force > 0.9*maxForce){
				cpSpaceRemoveConstraint(space, joint);
				cpConstraintFree(joint);
				breakableJoints[i] = NULL;
			}
		}
	}
}

static cpSpace *
init(void)
{
	space = cpSpaceNew();
	cpSpaceSetIterations(space, 30);
	cpSpaceSetGravity(space, cpv(0, -100));
	cpSpaceSetSleepTimeThreshold(space, 0.5f);
	
	cpBody *body, *staticBody = cpSpaceGetStaticBody(space);
	cpShape *shape;
	
	// Create segments around the edge of the screen.
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(-320,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(320,-240), cpv(320,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(320,-240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,240), cpv(320,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);
	
	cpFloat mass = 1;
	cpFloat width = 20;
	cpFloat height = 30;
	
	cpFloat spacing = width*0.3;
	
	// Add lots of boxes.
	for(int i=0; i<CHAIN_COUNT; i++){
		cpBody *prev = NULL;
		
		for(int j=0; j<LINK_COUNT; j++){
			cpVect pos = cpv(40*(i - (CHAIN_COUNT - 1)/2.0), 240 - (j + 0.5)*height - (j + 1)*spacing);
			
			body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, width, height)));
			cpBodySetPos(body, pos);
			
			shape = cpSpaceAddShape(space, cpBoxShapeNew(body, width, height));
			cpShapeSetFriction(shape, 0.8f);
			
			cpFloat breakingForce = 80000;
			
			cpConstraint *constraint = NULL;
			if(prev == NULL){
				constraint = cpSpaceAddConstraint(space, cpSlideJointNew(body, staticBody, cpv(0, height/2), cpv(pos.x, 240), 0, spacing));
			} else {
				constraint = cpSpaceAddConstraint(space, cpSlideJointNew(body, prev, cpv(0, height/2), cpv(0, -height/2), 0, spacing));
			}
			
			cpConstraintSetMaxForce(constraint, breakingForce);
			breakableJoints[i + j*CHAIN_COUNT] = constraint;
			
			prev = body;
		}
	}
	
	cpFloat radius = 15.0f;
	body = cpSpaceAddBody(space, cpBodyNew(10.0f, cpMomentForCircle(10.0f, 0.0f, radius, cpvzero)));
	cpBodySetPos(body, cpv(0, -240 + radius+5));
	cpBodySetVel(body, cpv(0, 300));

	shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
	cpShapeSetElasticity(shape, 0.0f);
	cpShapeSetFriction(shape, 0.9f);
	
	return space;
}

static void
destroy(void)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo Chains = {
	"Breakable Chains",
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
