#define BOOST_TEST_MODULE Citrus Engine Tests
#include <boost/test/included/unit_test.hpp>

#include "Citrus.h"

#include <string>

int dummyCount = 0;
		
void Dummy(CitrusEngine::Event& event){
		dummyCount++;
}

class DummyEvent : public CitrusEngine::Event {
public:
	std::string GetType() { return "DummyEvent"; }
};

BOOST_AUTO_TEST_CASE(TestEventConsumers) {
		CitrusEngine::EventManager em{};

		CitrusEngine::EventConsumer* dummyConsumer = new CitrusEngine::EventConsumer(Dummy);

		em.SubscribeConsumer("DummyEvent", dummyConsumer);

		DummyEvent dummyEvent{};

		em.Dispatch(dummyEvent);

		BOOST_TEST(dummyCount == 1);

		em.UnsubscribeConsumer("DummyEvent", dummyConsumer);

		DummyEvent dummyEvent2{};

		em.Dispatch(dummyEvent2);

		BOOST_TEST(dummyCount == 1);

		delete dummyConsumer;
}