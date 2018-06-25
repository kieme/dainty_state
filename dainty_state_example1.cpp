#include <iostream>
#include <assert.h>

#include "dainty_state.h"

using namespace dainty::state;

////////////////////////// design phase ////////////////////////////////

// which states are required
enum t_states { STATE1 = 0, STATE2, STATE3, STATE4, PRE };

// what data must be shared among them and the statemachine
struct data { };

// what interface of functions are required and must be implemented
struct interface {
  virtual ~interface() { }
  virtual t_states foo() = 0;
};

//////////////// choose statemachine framework  ////////////////////////

typedef t_traits<t_states, data, interface> my_traits;
typedef my_traits::t_state                  state;
typedef my_traits::t_statemachine           statemachine;

//////////////////// implementation phase //////////////////////////////

struct state1 final : state
{
  state1(data& d) : state(STATE1, t_user_ref(d)) { }
  virtual t_sid entry_point() override { std::cout << "state1::entry_point" << std::endl; return no_transition(); }
  virtual void  exit_point()  override { std::cout << "state1::exit_point"  << std::endl; }
  virtual t_sid foo()         override { std::cout << "state1::foo"         << std::endl; return request_transition(STATE2); }
};

struct state2 final : state
{
  state2(data& d) : state(STATE2, t_user_ref(d)) { }
  virtual t_sid entry_point() override { std::cout << "state2::entry_point" << std::endl; return no_transition(); }
  virtual void  exit_point()  override { std::cout << "state2::exit_point"  << std::endl; }
  virtual t_sid foo()         override { std::cout << "state2::foo"         << std::endl; return request_transition(STATE3); }
};

struct state3 final : state
{
  state3(data& d) : state(STATE3, state::t_user_ref(d)) { }
  virtual t_sid entry_point() override { std::cout << "state3::entry_point" << std::endl; return no_transition(); }
  virtual void  exit_point()  override { std::cout << "state3::exit_point"  << std::endl; }
  virtual t_sid foo()         override { std::cout << "state3::foo"         << std::endl; return request_transition(STATE4); }
};

struct state4 final : state
{
  state4(data& d) : state(STATE4, state::t_user_ref(d)) { }
  virtual t_sid entry_point() override { std::cout << "state4::entry_point" << std::endl; return no_transition(); }
  virtual void  exit_point()  override { std::cout << "state4::exit_point"  << std::endl; }
  virtual t_sid foo()         override { std::cout << "state4::foo"         << std::endl; return request_transition(STATE1); }
};

struct sm : statemachine
{
  sm(data& d) : statemachine(PRE, statemachine::t_user_ref(d)),
                s1_(d), s2_(d), s3_(d), s4_(d)
  {
    states_[STATE1] = &s1_;
    states_[STATE2] = &s2_;
    states_[STATE3] = &s3_;
    states_[STATE4] = &s4_;

    assert(start(STATE1) == STATE1);
  }

  ~sm()        { stop(); }

  void reset() { assert(restart(STATE1) == STATE1); }

  virtual t_states foo          ()              override { return do_transition(get_current()->foo()); }

  virtual t_states initial_point(t_states s)    override { std::cout << "initial action" << std::endl; return s; }
  virtual void     final_point  ()              override { std::cout << "final action" << std::endl;             }

  virtual p_state  get_state    (t_sid i)       override { return states_[i]; }
  virtual p_cstate get_state    (t_sid i) const override { return states_[i]; }

  state1 s1_;
  state2 s2_;
  state3 s3_;
  state4 s4_;
  state* states_[PRE];
};

// function uses the statemachine
struct function
{
  function() : sm_(data_)   { }

  void foo()   { sm_.foo();   }
  void reset() { sm_.reset(); }

private:
  data data_;
  sm   sm_;
};

// execution
int main()
{
  function f;
  f.foo();
  f.foo();

  f.reset();

  f.foo();
  f.foo();

  return 0;
}
