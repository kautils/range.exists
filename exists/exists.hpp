#ifndef FLOW_RANGE_EXISTS_EXISTS_EXISTS_HPP


#include "kautil/algorithm/btree_search/btree_search.hpp"
namespace kautil{
namespace range{

template<typename preference_t>
struct exists{
    using value_type = typename preference_t::value_type;
    using offset_type = typename preference_t::offset_type;
    
    exists(preference_t * pref) : pref(pref){}
    virtual ~exists(){}
    
    bool exec(value_type from, value_type to){
        auto is_contained = [](auto const& in)->bool{
            return
                !in.direction
             | (bool(in.nearest_pos%(sizeof(value_type)*2))&(in.direction<0)) 
             | (!bool(in.nearest_pos%(sizeof(value_type)*2)))&(in.direction>0);
        };

        auto bt = kautil::algorithm::btree_search{pref};
        auto i0 = bt.search(from,false);
        auto i1 = bt.search(to,false);
        auto i0_is_contained = !i0.nan*is_contained(i0);
        auto i1_is_contained = !i1.nan*is_contained(i1);
        auto contained = ((from==to)&i0_is_contained)|(i0_is_contained&i1_is_contained);
        auto size_check = (sizeof(value_type)>=(-i0.nearest_pos+i1.nearest_pos));
        return contained&size_check; 
    }
    
    
private:
    preference_t * pref = nullptr;
    
};



} //namespace range{
} //namespace kautil{


#endif


