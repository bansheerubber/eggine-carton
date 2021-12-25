#pragma once

namespace carton {
	class Metadata;

	namespace metadata {
		class QueryObject {
			public:
				QueryObject() {}
				QueryObject(class QueryList* list) {
					this->list = list;
				}
				virtual ~QueryObject() {}
				
				virtual bool test(carton::Metadata* metadata) = 0;
			
			private:
				class QueryList* list;
		};
	};
};
