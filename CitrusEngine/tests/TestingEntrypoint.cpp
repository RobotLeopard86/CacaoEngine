#define BOOST_TEST_MODULE Citrus Engine Tests
#include <boost/test/included/unit_test.hpp>

#include "Citrus.h"

int dummyCount = 0;
		
void Dummy(CitrusEngine::Event& event){
		dummyCount++;
}

BOOST_AUTO_TEST_CASE(TestEventManagerUnsubscription) {
		CitrusEngine::EventManager em{};

		CitrusEngine::EventConsumer* dummyConsumer = new CitrusEngine::EventConsumer(Dummy);

		em.SubscribeConsumer("WindowClose", dummyConsumer);

		CitrusEngine::WindowCloseEvent dummyEvent{};

		em.Dispatch(dummyEvent);

		BOOST_TEST(dummyCount == 1);

		em.UnsubscribeConsumer("WindowClose", dummyConsumer);

		CitrusEngine::WindowCloseEvent dummyEvent2{};

		em.Dispatch(dummyEvent2);

		BOOST_TEST(dummyCount == 1);

		delete dummyConsumer;
}