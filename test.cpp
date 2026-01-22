#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "structure_prototype.h"

namespace {
class MockA :public ComponentA {
 public:
  explicit MockA(std::unique_ptr<ComponentACallbacks> callbacks) : ComponentA(std::move(callbacks)) {}
  MOCK_METHOD(void, SomeFuncOfA, (), (override));
};
class MockB :public ComponentB {
 public:
  explicit MockB(std::unique_ptr<ComponentBCallbacks> callbacks) : ComponentB(std::move(callbacks)) {}
  MOCK_METHOD(void, SomeFuncOfB, (), (override));
};
class TestableProcess : public Process {
 public:
  using AFactory = Process::AFactory;
  using BFactory = Process::BFactory;
  TestableProcess(const AFactory& a_factory, const BFactory& b_factory)
      : Process(a_factory, b_factory) {}
};
class ProcessTests : public ::testing::Test {
 protected:
  ProcessTests()
      : sut_([this](std::unique_ptr<ComponentACallbacks> cb) { return AFactory(std::move(cb)); },
             [this](std::unique_ptr<ComponentBCallbacks> cb) { return BFactory(std::move(cb)); }) {}
  void SetUp() override {
  }
  std::unique_ptr<ComponentA> AFactory(std::unique_ptr<ComponentACallbacks> callbacks) {
    // 'Observer Pointer' pattern, used to 'observer' callbacks and Mocks ignore static code
    // analysis.
    raw_a_callbacks_ = callbacks.get();
    auto mock_a = std::make_unique<MockA>(std::move(callbacks));
    raw_a_ = mock_a.get();
    return mock_a;
  }
  std::unique_ptr<ComponentB> BFactory(std::unique_ptr<ComponentBCallbacks> callbacks) {
    // 'Observer Pointer' pattern, used to 'observer' callbacks and Mocks ignore static code
    // analysis.
    raw_b_callbacks_ = callbacks.get();
    auto mock_b = std::make_unique<MockB>(std::move(callbacks));
    raw_b_ = mock_b.get();
    return mock_b;
  }

  ComponentACallbacks* raw_a_callbacks_{nullptr};
  MockA* raw_a_{nullptr};
  ComponentBCallbacks* raw_b_callbacks_{nullptr};
  MockB* raw_b_{nullptr};
  TestableProcess sut_;
};
}  // namespace

// for example, when a client dies, we want to confirm we ask the pose process to stop generating
// data
TEST_F(ProcessTests, ASomeFuncTriggersBCallback) {
  // Expect some function to be called
  EXPECT_CALL(*raw_b_, SomeFuncOfB());

  // trigger the test event
  raw_a_callbacks_->SomeAEvent();
}
//for example suppose we have new pose data given to us by the callback, we want to confirm this
//data is passed on to the clients.
TEST_F(ProcessTests, BSomeFuncTriggersACallback) {
  EXPECT_CALL(*raw_a_, SomeFuncOfA());

  // trigger the test event
  raw_b_callbacks_->SomeBEvent();
}
