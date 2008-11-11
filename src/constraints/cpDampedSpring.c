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

//#include <stdlib.h>
//#include <math.h>

#include <stdlib.h>
#include <math.h>

#include "../chipmunk.h"
#include "util.h"

static void
preStep(cpDampedSpring *spring, cpFloat dt, cpFloat dt_inv)
{
	cpBody *a = spring->constraint.a;
	cpBody *b = spring->constraint.b;
	
	spring->r1 = cpvrotate(spring->anchr1, a->rot);
	spring->r2 = cpvrotate(spring->anchr2, b->rot);
	
	cpVect delta = cpvsub(cpvadd(b->p, spring->r2), cpvadd(a->p, spring->r1));
	cpFloat dist = cpvlength(delta);
	spring->n = cpvmult(delta, 1.0f/(dist ? dist : INFINITY));
	
	// calculate mass normal
	spring->nMass = 1.0f/k_scalar(a, b, spring->r1, spring->r2, spring->n);

	spring->dt = dt;
	spring->target_vrn = 0.0f;

	// apply spring force
	cpFloat f_spring = (spring->restLength - dist)*spring->stiffness;
	apply_impulses(a, b, spring->r1, spring->r2, cpvmult(spring->n, f_spring*dt));
}

static void
applyImpulse(cpDampedSpring *spring)
{
	cpBody *a = spring->constraint.a;
	cpBody *b = spring->constraint.b;
	
	cpVect n = spring->n;
	cpVect r1 = spring->r1;
	cpVect r2 = spring->r2;

	// compute relative velocity
	cpFloat vrn = normal_relative_velocity(a, b, r1, r2, n) - spring->target_vrn;
	
	// compute velocity loss from drag
	// not 100% certain this is derived correctly, though it makes sense
	cpFloat v_damp = -vrn*(1.0f - exp(-spring->damping*spring->dt/spring->nMass));
	spring->target_vrn = vrn + v_damp;
	
	apply_impulses(a, b, spring->r1, spring->r2, cpvmult(spring->n, v_damp*spring->nMass));
}

static cpFloat
getImpulse(cpConstraint *constraint)
{
	return 0.0f;
}

const cpConstraintClass cpDampedSpringClass = {
	(cpConstraintPreStepFunction)preStep,
	(cpConstraintApplyImpulseFunction)applyImpulse,
	(cpConstraintGetImpulseFunction)getImpulse,
};

cpDampedSpring *
cpDampedSpringAlloc(void)
{
	return (cpDampedSpring *)malloc(sizeof(cpDampedSpring));
}

cpDampedSpring *
cpDampedSpringInit(cpDampedSpring *spring, cpBody *a, cpBody *b, cpVect anchr1, cpVect anchr2, cpFloat restLength, cpFloat stiffness, cpFloat damping)
{
	cpConstraintInit((cpConstraint *)spring, &cpDampedSpringClass, a, b);
	
	spring->anchr1 = anchr1;
	spring->anchr2 = anchr2;
	
	spring->restLength = restLength;
	spring->stiffness = stiffness;
	spring->damping = damping;
	
	return spring;
}

cpConstraint *
cpDampedSpringNew(cpBody *a, cpBody *b, cpVect anchr1, cpVect anchr2, cpFloat restLength, cpFloat stiffness, cpFloat damping)
{
	return (cpConstraint *)cpDampedSpringInit(cpDampedSpringAlloc(), a, b, anchr1, anchr2, restLength, stiffness, damping);
}

void
cpDampedSpringGetProperties(cpConstraint *constraint, cpFloat *restLength, cpFloat *stiffness, cpFloat *damping)
{
	CHECK_CLASS(constraint, &cpDampedSpringClass);
	cpDampedSpring *spring = (cpDampedSpring *)constraint;

	if(restLength) (*restLength) = spring->restLength;
	if(stiffness)  (*stiffness)  = spring->stiffness;
	if(damping)    (*damping)    = spring->damping;
}

void
cpDampedSpringSetProperties(cpConstraint *constraint, cpFloat restLength, cpFloat stiffness, cpFloat damping)
{
	CHECK_CLASS(constraint, &cpDampedSpringClass);
	cpDampedSpring *spring = (cpDampedSpring *)constraint;
	
	spring->restLength = restLength;
	spring->stiffness  = stiffness;
	spring->damping    = damping;
}

