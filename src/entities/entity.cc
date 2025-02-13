#include "entity.hh"
#include "data.hh"
#include "effect.hh"
#include "game.hh"
//#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <iostream>
#include <math.h>
#define DRAG_CONSTANT 5.0f

Entity::Entity(GLuint type, const glm::vec3& position, Game* game, float speed, float mass)
	:GameObject(type, position, game){
	acceleration_ = glm::vec3(0);
	velocity_ = glm::vec3(0);
	speed_ = speed;
	hitbox_ =  Hitbox(&position_, scale_*0.6);
	mass_ = mass;

	explosion_effect_ = EFFECT_EXPLOSION;
}

void Entity::Update(float dt){

	//AIR RESISTANCE
	//Proptional to v^2 * some constants such as fluid density and area...
	//Force applied in the opposite direction to velocity
	float v = glm::length(velocity_);
	if(glm::length(velocity_) > 0)
		acceleration_ -= (glm::normalize(velocity_) * (v*v) * DRAG_CONSTANT);

	//Velocity is accumulated while acceleration denotes our rate of change of velocity
	//at this exact time step. acceleration = dv/dt 
	velocity_ += (acceleration_ * dt);
	position_ += velocity_;
	acceleration_ = glm::vec3(0);
	
	//Update all children
	for(std::vector<GameObject*>::iterator child = children_.begin(); child != children_.end();) {
		(*child)->Update(dt);
		if((*child)->IsDestroyed()) {
			children_.erase(child);
			//Hopefully this gets rid of item pointers too may have to check if item too if there is a seg fault
		} else {
			++child;
		}
	}
	for(std::vector<Item*>::iterator i = items_.begin(); i != items_.end();) {
		if((*i)->IsDestroyed()) {
			items_.erase(i);
		} else {
			++i;
		}
	}
}

void Entity::SetEffect(GLuint eff){
	explosion_effect_ = eff;
}

void Entity::AddWeapon(Weapon *wpn){
	wpn->Attach(&position_, &angle_);
	AddChild(wpn);
	weapons_.push_back(wpn);
}

void Entity::AddItem(Item* item)
{
	std::vector<Item*> items_to_update;
	item->Activate();
	AddChild(item);
	items_.push_back(item);
	//Not gunna lie this is kinda jank this is how I update the positions of the items on the fly when something is added. 
	//Could possibly just have the item randomly guess a probably open position to avoid O(n) computation tho but idk this works
	items_to_update.push_back(item);
	for (Item* i : items_) {
		if(i->GetRadius() == item->GetRadius()) {
			items_to_update.push_back(i);
		}
	}
	int count = 1;
	for (Item* i : items_to_update) {
		i->SetRadiusProgress(2*count*M_PI/(items_to_update.size()-1));
		++count;
	}
}

void Entity::Thrust(float dt){
	acceleration_ = GetAngleVector() * speed_ * (dt*50);
	//acceleration_ = glm::rotate(glm::vec3(0, speed_, 0), angle_, glm::vec3(0,0,1))*(dt*45);
}

void Entity::Break(){
	//Reduce acceleration and velocity
	acceleration_ *= 0.98;
}

void Entity::Shoot(){
	for(Weapon* w : weapons_){
		w->Shoot();
	}
}

void Entity::Turn(int d, float dt){
	if(d > 0){
		angle_ += (4.0f  *dt);
	}else {
		angle_ -= (4.0f * dt);
	}
}

void Entity::TurnTowards(glm::vec3 &t, float dt){
	float perp_angle = angle_ + M_PI/2 - 0.15;
	glm::vec3 perp = glm::rotate(glm::vec3(0,1,0), perp_angle, glm::vec3(0,0,1));
	float dp = glm::dot(perp, t);
	float a = acos(dp/(glm::length2(perp) * glm::length2(t)));
	if(a > 0.2)
		Turn(dp, dt);
	else
		LookAtPoint(t);
}

void Entity::LookAtPoint(glm::vec3 target)
{
	float angle = glm::atan(target.y - position_.y, target.x - position_.x);
	angle_ = angle - M_PI_2;
	for(Weapon* w : weapons_){
		w->AimAt(target);
	}
}

void Entity::SetScale(float s){
	scale_ = s;
	hitbox_.SetRadius(s/2);
}

glm::vec3& Entity::GetVelocity() {
	return velocity_;
}

float Entity::GetMass(){
	return mass_;
}

void Entity::SetVelocity(const glm::vec3 &v){
	velocity_ = v;
}

void Entity::SetPosition(const glm::vec3 &p){
	position_ = p;
}

Hitbox& Entity::GetHitbox(){
	return hitbox_;
}

bool Entity::CheckCollision(Hitbox& hbox){
	return hitbox_.CheckCollision(hbox);
}

void Entity::Explode(){
	if(!RaiseEvent("block")){
		game_->SpawnExplosion(explosion_effect_, position_, scale_);
		destroyed_ = true;
	}
}

bool Entity::RaiseEvent(const std::string& event) {
	for(Item* i : items_) {
		if(i->Event(event)) {
			return true;
		}
	}
	return false;
}
bool Entity::CheckShield(){
	
	/*for(int i = 0; i < children_.size(); i++){
		if(children_[i]->GetType() == SHIELD){
			delete children_[i];
			children_.erase(children_.begin() + i);
			return true;
		}
	}
	return false;*/
	return false;
}

void Entity::PowerUp(){
	draw_inverted_ = true;
	for(Weapon* w : weapons_){
		w->AddShootingAngle(M_PI/8);
		w->AddShootingAngle(-M_PI/8);
	}
}

