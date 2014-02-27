// $Id: inscount2.cpp 2286 2013-09-19 18:40:30Z hillj $

#include "pin++/Callback.h"
#include "pin++/Pintool.h"
#include "pin++/Buffer.h"
#include "pin++/Trace_Instrument.h"

#include <fstream>
#include <list>

class docount : public OASIS::Pin::Callback <docount (void)>
{
public:
  docount (void)
    : count_ (0),
      increment_ (0)
  {

  }

  void handle_analyze (void)
  {
    this->count_ += this->increment_;
  }

  void increment (UINT64 incr)
  {
    this->increment_ = incr;
  }

  UINT64 count (void) const
  {
    return this->count_;
  }

private:
  UINT64 count_;
  UINT64 increment_;
};

class Trace : public OASIS::Pin::Trace_Instrument <Trace>
{
public:
  void handle_instrument (const OASIS::Pin::Trace & trace)
  {
    // Allocate a callback for each BBL.
    item_type item (trace.num_bbl ());
    item_type::iterator callback = item.begin ();

    using OASIS::Pin::Bbl;

    for (OASIS::Pin::Bbl bbl : trace)
    {
      callback->increment (bbl.ins_count ());
      bbl.insert_call (IPOINT_ANYWHERE, callback ++);
    }

    this->traces_.push_back (item);
  }

  UINT64 count (void) const
  {
    UINT64 count = 0;

    for (auto trace : this->traces_)
      for (auto item : trace)
        count += item.count ();

    return count;
  }

private:
  typedef OASIS::Pin::Buffer <docount> item_type;
  typedef std::list <item_type> list_type;

  list_type traces_;
};

class inscount : public OASIS::Pin::Tool <inscount>
{
public:
  inscount (void)
  {
    this->enable_fini_callback ();
  }

  void handle_fini (INT32)
  {
    std::ofstream fout ("inscount.out");
    fout <<  "Count " << this->trace_.count () << std::endl;

    fout.close ();
  }

private:
  Trace trace_;

  /// @{ KNOBS
  static KNOB <string> outfile_;
  /// @}
};

KNOB <string> inscount::outfile_ (KNOB_MODE_WRITEONCE, "pintool", "o", "inscount.out", "specify output file name");

DECLARE_PINTOOL (inscount);
