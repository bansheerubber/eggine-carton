#include "egg.h"

#include "carton.h"

carton::EggContents::EggContents() {

}

carton::EggContents::EggContents(Carton* carton) {
	this->carton = carton;
	carton->contents.push_back(this);
}

carton::EggContents::~EggContents() {
	
}
