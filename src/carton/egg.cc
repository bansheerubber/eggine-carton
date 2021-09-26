#include "egg.h"

#include "carton.h"

carton::EggContents::EggContents() {

}

carton::EggContents::EggContents(Carton* carton) {
	this->carton = carton;
	carton->contents.insert(this);
}

carton::EggContents::~EggContents() {
	this->carton->contents.erase(this);
}
