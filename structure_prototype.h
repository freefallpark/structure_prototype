// Copyright (c) 2026, Ultradent Products Inc. All rights reserved.

#ifndef PROJECT_STRUCTURE_PROTOTYPE_STRUCTURE_PROTOTYPE_H_
#define PROJECT_STRUCTURE_PROTOTYPE_STRUCTURE_PROTOTYPE_H_

#include <chrono>
#include <functional>
#include <memory>
#include <thread>

// Some Generic component 'A' ----------------------------------------------------------------------
// This could be a communication server or client for example.

// Callbacks allow the scope that owns the component to define behavior when certain events occur
class ComponentACallbacks {
 public:
  virtual ~ComponentACallbacks() = default;

  // For example, a server detects a client is lost
  virtual void SomeAEvent() = 0;
};

// Component A defines the generic interface for components of type 'A'
class ComponentA {
 public:
  // Component constructs with some callbacks; unique_ptr communicates it owns when they're
  // called
  ComponentA() = delete;
  explicit ComponentA(std::unique_ptr<ComponentACallbacks> callbacks)
      : callbacks_(std::move(callbacks)) {}
  virtual ~ComponentA() = default;

  // Components of type 'A' are required to do certain things.
  virtual void SomeFuncOfA() = 0;

 protected:
  // Protected access allows implementations to use the callbacks object
  std::unique_ptr<ComponentACallbacks> callbacks_{nullptr};
};

// Some specific implementation of an 'A' component
class ComponentAImpl : public ComponentA {
 public:
  // Pass through a callback object, allowing owning code to define the callback(s), while allowing
  // this component to define WHEN it's called
  explicit ComponentAImpl(std::unique_ptr<ComponentACallbacks> callbacks)
      : ComponentA(std::move(callbacks)) {}
  ~ComponentAImpl() override = default;
  // Define how this implementation achieves the required task 'SomeFuncOfA'
  void SomeFuncOfA() override {}
};

// Some Generic Component 'B' ----------------------------------------------------------------------
// this could be, for example, a Pose Estimator
// Define the events that can be 'triggered' by component 'B'
class ComponentBCallbacks {
 public:
  virtual ~ComponentBCallbacks() = default;
  // For example, a new pose has been calculated.
  virtual void SomeBEvent() = 0;
};
// defines the interface for components of type 'B'
class ComponentB {
 public:
  // Require callback definitions to be injected into components of type 'B'
  ComponentB() = delete;
  explicit ComponentB(std::unique_ptr<ComponentBCallbacks> callbacks)
      : callbacks_(std::move(callbacks)) {}
  virtual ~ComponentB() = default;

  //Components of type be are required to define how they accomplish certain tasks
  virtual void SomeFuncOfB() = 0;

 protected:
  // Protected access allows implementations to use the callbacks object
  std::unique_ptr<ComponentBCallbacks> callbacks_{nullptr};
};
// Some implementation of 'B'
class BImpl : public ComponentB {
 public:
  // Calling scope defines callbacks, while implementation determines when they're called.
  explicit BImpl(std::unique_ptr<ComponentBCallbacks> callbacks)
      : ComponentB(std::move(callbacks)) {}
  ~BImpl() override = default;
  // Definition of how component 'B' achieves its required task
  void SomeFuncOfB() override {}
};

// Some process that uses a ComponentA and a ComponentB ----------------------------------------
// This could be, for example, the pose estimation process that uses a pose estimator and
// publishes it for all of its clients. We want the process to define what happens when 'A' and 'B'
// events occur while allowing A and B components to trigger them.
class Process {
 public:
  // By default, the Process chooses its own components and gets to define its own callback handling
  // this creates a circular dependency. A process needs the components, the components need the
  // handlers, and the handlers need the process.
  Process()
      : a_(std::make_unique<ComponentAImpl>(std::make_unique<ACallbackHandler>(*this))),
        b_(std::make_unique<BImpl>(std::make_unique<BCallbackHandler>(*this))) {}

  // we could also privatize the above constructor and make factory methods instead.  One for each
  // desired component implementation grouping

  // Some very basic/limited API since this will be in main.
  [[nodiscard]] int Run(volatile __sig_atomic_t stop_signal) const {
    Init();
    while (stop_signal == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
  };

 protected:  // here, protected methods are exposed for testing purposes.
  // The circular dependencies only really need to be dealt with during testing. By using lambdas,
  // we can allow mocking of the components while not changing Process A functionality (still
  // allowing it to define the callback handling). The testing structure can access components and
  // trigger callbacks using the observer pointer pattern.
  using AFactory =
      std::function<std::unique_ptr<ComponentA>(std::unique_ptr<ComponentACallbacks> cb)>;
  using BFactory =
      std::function<std::unique_ptr<ComponentB>(std::unique_ptr<ComponentBCallbacks> cb)>;
  Process(const AFactory& a_factory, const BFactory& b_factory)
      : a_(a_factory(std::make_unique<ACallbackHandler>(*this))),
        b_(b_factory(std::make_unique<BCallbackHandler>(*this))) {}

 private:
  // Process gets to define what occurs during Callback events
  // The handlers own a reference to its parent process. This gives the handlers full access to
  // their parent process's components, including private. Because these private objects can only be
  // created inside 'Process', there are no lifetime issues.
  class ACallbackHandler : public ComponentACallbacks {
   public:
    explicit ACallbackHandler(Process& process) : process_(process) {}
    // Suppose this represents losing a client who had requested pose data. we could stop generating
    // the data
    void SomeAEvent() override { process_.b_->SomeFuncOfB(); }

   private:
    Process& process_;
  };
  class BCallbackHandler : public ComponentBCallbacks {
   public:
    explicit BCallbackHandler(Process& process) : process_(process) {}
    // Suppose this method represents new incoming pose data, we could choose to share that data
    // with a client using component 'a'
    void SomeBEvent() override { process_.a_->SomeFuncOfA(); }

   private:
    Process& process_;
  };

  // we could use components freely in process class code as well.
  void Init() const {
    a_->SomeFuncOfA();
    b_->SomeFuncOfB();
  };

  // unique ownership communicates that these are OWNED by the process, no one else.
  // for testing using the factory lambdas allows the test structure to get around this by using
  // observer pointers
  std::unique_ptr<ComponentA> a_{nullptr};
  std::unique_ptr<ComponentB> b_{nullptr};
};

#endif  // PROJECT_STRUCTURE_PROTOTYPE_STRUCTURE_PROTOTYPE_H_
