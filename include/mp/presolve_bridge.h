#ifndef PRESOLVE_BRIDGE_H
#define PRESOLVE_BRIDGE_H

#include <vector>

#include "presolve_node.h"


namespace mp {

namespace pre {

/// Index range in a bridge
using BridgeIndexRange = IndexRange;

/// Declare Presolver
class Presolver;

/// Bridge interface
///
/// A bridge is an array of value converters between nodes.
/// All converters are of the same type
class BasicBridge {
public:
  /// Constructor
  BasicBridge(Presolver& pre) : presolver_(pre) { }
  /// Virtual destructor
  virtual ~BasicBridge() = default;

  /// The below pre- / postsolves work on a range of bridge entries
  /// Postsolves should usually loop the range backwards

  /// Presolve solution (primal + dual)
  virtual void PresolveSolution(BridgeIndexRange ) = 0;
  /// Postsolve solution (primal + dual)
  virtual void PostsolveSolution(BridgeIndexRange ) = 0;

  /// Presolve basis (primal + dual)
  virtual void PresolveBasis(BridgeIndexRange ) = 0;
  /// Postsolve solution (primal + dual)
  virtual void PostsolveBasis(BridgeIndexRange ) = 0;

protected:
  /// Add a range of bridge entries to the Presolver's list
  /// Every derived bridge should call either of the next two
  /// whenever adding a bridge entry
  void RegisterBridgeIndex(int i)
  { RegisterBridgeIndexRange( {i, i+1} ); }
  void RegisterBridgeIndexRange(BridgeIndexRange );

private:
  Presolver& presolver_;
};


/// Bridge range: range of conversion specifiers of certain type
/// The bridge is specified as well
struct BridgeRange {
  /// Try & extend the range
  /// @return true iff extension worked,
  /// otherwise the caller would have to add the new range
  /// separately
  bool ExtendRange(BridgeRange br) {
    if (&b_ == &br.b_) {                  // same bridge
      if (ir_.end == br.ir_.beg) {        // and consecutive ranges
        ir_.end = br.ir_.end;
        return true;
      }
    }
    return false;
  }

  BasicBridge& b_;
  BridgeIndexRange ir_;
};


/// A specific bridge: each entry just copies a range of values
class CopyBridge : public BasicBridge {
public:
  /// Constructor
  CopyBridge(Presolver& pre) : BasicBridge(pre) { }

  /// Single bridge entry
  using BridgeEntry = std::pair<NodeRange, NodeRange>;

  /// Add entry
  void AddEntry(BridgeEntry be) {
    entries_.push_back(be);
    RegisterBridgeIndex(entries_.size()-1);
  }

  /// Presolve solution (primal + dual)
  void PresolveSolution(BridgeIndexRange ir) override
  { CopySrcDest(ir); }
  /// Postsolve solution (primal + dual)
  void PostsolveSolution(BridgeIndexRange ir) override
  { CopyDestSrc(ir); }

  /// Presolve basis (primal + dual)
  void PresolveBasis(BridgeIndexRange ir) override
  { CopySrcDest(ir); }
  /// Postsolve solution (primal + dual)
  void PostsolveBasis(BridgeIndexRange ir) override
  { CopyDestSrc(ir); }

protected:
  /// Copy src -> dest for index range ir
  void CopySrcDest(BridgeIndexRange ir) {
    for (int i=ir.beg; i!=ir.end; ++i) {
      const auto& br = entries_[i];
      assert(br.first.ir_.size() == br.second.ir_.size());
      br.first.vn_.Copy(br.first.ir_, br.second.vn_, br.second.ir_.beg);
    }
  }
  /// Copy src <- dest for index range ir
  void CopyDestSrc(BridgeIndexRange ir) {
    for (int i=ir.beg; i!=ir.end; ++i) {
      const auto& br = entries_[i];
      assert(br.first.ir_.size() == br.second.ir_.size());
      br.second.vn_.Copy(br.second.ir_, br.first.vn_, br.first.ir_.beg);
    }
  }

private:
  std::vector<BridgeEntry> entries_;
};

} // namespace pre

} // namespace mp

#endif // PRESOLVE_BRIDGE_H
