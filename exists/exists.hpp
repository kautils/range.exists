#ifndef FLOW_RANGE_EXISTS_EXISTS_EXISTS_HPP

#include <cstdint>
#include <vector>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>


template<typename premitive>
struct file_syscall_premitive{
    using value_type = premitive;
    using offset_type = long;

    int fd=-1;
    char * buffer = 0;
    offset_type buffer_size = 0;
    
    ~file_syscall_premitive(){ free(buffer); }
    offset_type block_size(){ return sizeof(value_type); }
    offset_type size(){ return lseek(fd,0,SEEK_END)- lseek(fd,0,SEEK_SET); }
    
    void read_value(offset_type const& offset, value_type ** value){
        lseek(fd,offset,SEEK_SET);
        ::read(fd,*value,sizeof(value_type));
    }
    
    bool write(offset_type const& offset, void ** data, offset_type size){
        lseek(fd,offset,SEEK_SET);
        return size==::write(fd,*data,size);
    }
    
    // api may make a confusion but correct. this is because to avoid copy of value(object such as bigint) for future.
    bool read(offset_type const& from, void ** data, offset_type size){
        lseek(fd,from,SEEK_SET);
        return size==::read(fd,*data,size);
    }
    
    bool extend(offset_type extend_size){ 
        return !ftruncate(fd,size()+extend_size);   
    }
    int shift(offset_type dst,offset_type src,offset_type size){
        if(buffer_size < size){
            if(buffer)free(buffer);
            buffer = (char*) malloc(buffer_size = size);
        }
        lseek(fd,src,SEEK_SET);
        auto read_size = ::read(fd,buffer,size);
        lseek(fd,dst,SEEK_SET);
        ::write(fd,buffer,read_size);
        return 0;
    }
    
    int flush_buffer(){ return 0; }
};

using file_syscall_16b_pref= file_syscall_premitive<uint64_t>;
using file_syscall_16b_f_pref= file_syscall_premitive<double>;


#include "kautil/algorithm/btree_search/btree_search.hpp"
#include <set>



namespace kautil{
namespace range{

template<typename preference_t>
struct exists{
    using value_type = typename preference_t::value_type;
    using offset_type = typename preference_t::offset_type;
    
    exists(preference_t * pref) : pref(pref){}
    ~exists(){}
    
    bool exec(value_type from, value_type to){
        auto is_adjust_diff =[](auto const& i0,auto from,auto diff)->bool{
            /* (np-diff <= input <= np) or (np <= input <= np+diff)
             then adjust */
            return 
                 ((i0.nearest_value-diff <= from) &(from <= i0.nearest_value))
                +((i0.nearest_value <= from) &(from <= i0.nearest_value+diff));
        };
        
        auto i0_is_exact = [](auto const& i0)->bool{ return (!i0.direction)& !bool(i0.nearest_pos%(sizeof(value_type)*2)); };
        auto i1_is_exact = [](auto const& i1)->bool{ return (!i1.direction)&  bool(i1.nearest_pos%(sizeof(value_type)*2)); };
        auto is_contained = [](auto const& in)->bool{
            // conditions 'is_exact' can not be contained inside is_contained because there are different between i0 and i1. 
            return
               (bool(in.nearest_pos%(sizeof(value_type)*2))&(in.direction<0)) 
             | (!bool(in.nearest_pos%(sizeof(value_type)*2)))&(in.direction>0);
        };

        // todo : consider nan
        auto bt = kautil::algorithm::btree_search{pref};
            // there are 4 patterns (exact(2)) * (contained(2))
                //exact
                //contained
        auto i0 = bt.search(from,false);
        auto i1 = bt.search(to,false);
        
        {
            // if adjust then direction is 0. 
            auto i0_is_adjust =is_adjust_diff(i0,from,diff); 
            i0.direction *= !i0_is_adjust;
            from=
                 !i0_is_adjust*from
                +i0_is_adjust*i0.nearest_value;
            
            auto i1_is_adjust =is_adjust_diff(i1,to,diff); 
            i1.direction *= !is_adjust_diff(i1,to,diff);
            to = 
                 !i1_is_adjust*to
                +i1_is_adjust*i1.nearest_value;
        }
    
        auto test0 = is_contained(i0);
        auto test1 = is_contained(i1);
        
        auto contained = (is_contained(i0)|i0_is_exact(i0)) & (is_contained(i1)|i1_is_exact(i1));
        auto size_check = (sizeof(value_type)<=(-i0.nearest_pos+i1.nearest_pos));
        return contained&size_check; 
    }
    
    void set_diff(value_type v){ diff = v; }
    
private:
    uint64_t diff = 1;
    preference_t * pref = nullptr;
    
};



} //namespace range{
} //namespace kautil{



int tmain_kautil_range_exsits_interface() {
    
    using value_type = uint64_t;
    using offset_type = long;
    auto step = 10;
    auto data = std::vector<value_type>();{
        for(auto i = 0; i < 100; i+=2){
             data.push_back(i*step+step);
             data.push_back(data.back()+step);
        }
    }
    
    auto f_ranges = fopen("tmain_kautil_range_exsits_interface.cache","w+b");
    auto written = fwrite(data.data(),sizeof(value_type),data.size(),f_ranges);
    fflush(f_ranges);
    {
        
        auto fd = fileno(f_ranges);
        printf("file size : %ld\n",lseek(fd,0,SEEK_END)-lseek(fd,0,SEEK_SET));
        
        auto pref = file_syscall_16b_pref{.fd=fd};
        auto bt = kautil::algorithm::btree_search{&pref};
    
        //test info
        auto min = data.front();
        auto max = data.back();
        srand((uintptr_t)&pref);
        
//        for(auto i = 0; i < 1000; ++i){
        // todo : consider nan
            auto diff = 1;
            //auto from=value_type{11},to=value_type{19}; // expect true
            //auto from=value_type{0},to=value_type{5}; // expect false
            //auto from=value_type{20},to=value_type{30}; // expect false
            auto from=value_type{31},to=value_type{35}; // expect false
            //auto from=value_type{30},to=value_type{40}; // expect false
            //auto from=value_type{1000},to=value_type{1010}; // expect false
            auto ext = kautil::range::exists{&pref};
            ext.set_diff(diff);
            auto res = ext.exec(from,to);
            
//            from = (rand()%max+min)/step*step;
//            to = (rand()%max+min)/step*step;
            // there are 4 patterns (exact(2)) * (contained(2))
                //exact
                //contained
            auto i0 = bt.search(from,false);
            auto i1 = bt.search(to,false);
        
            struct check_st{ 
                value_type v;offset_type pos; 
                bool operator!=(check_st const & l)const{ return pos != l.pos; }
                bool operator<(check_st const & l)const{ return pos < l.pos; }
            };
            auto check = std::set<check_st>{};
            auto lmb_set_check = [](auto & check,auto & pref,auto & i0 ){
                
                auto is_limit_l = i0.nearest_pos<sizeof(value_type);
                auto is_limit_r = i0.nearest_pos+2*sizeof(value_type)>(pref.size());
                auto i0_check_buf = value_type(0);
                auto i_buf_ptr = &i0_check_buf;
                
                auto l_pos = offset_type(!is_limit_l*(i0.nearest_pos-sizeof(value_type)));
                auto r_pos = offset_type(!is_limit_r*(i0.nearest_pos+sizeof(value_type)));
                pref.read_value(l_pos,&(i_buf_ptr=&i0_check_buf));
                check.insert({.v=i0_check_buf,.pos=l_pos});
                pref.read_value(offset_type(i0.nearest_pos),&(i_buf_ptr=&i0_check_buf));
                check.insert({.v=i0_check_buf,.pos=offset_type(i0.nearest_pos)});
                pref.read_value(r_pos,&(i_buf_ptr=&i0_check_buf));
                check.insert({.v=i0_check_buf,.pos=r_pos});
            };
            
            lmb_set_check(check,pref,i0);
            lmb_set_check(check,pref,i1);

            printf("from,to : {%lld,%lld}\n",from,to);fflush(stdout);
            {
                printf("related poses : ");fflush(stdout);
                for(auto & elem : check)printf("%lld(%ld) ", elem.v, elem.pos);
                printf("\n");
                fflush(stdout);
            }
            
            auto cur = check.begin();
            auto e = check.end();
            auto check_res = false;
            auto l = (const check_st*)0; 
            auto r = (const check_st*)0; 
            for(;;){
                l = &*cur;;
                if(++cur==e){ break;}
                r = &*cur;
                if(++cur==e){ break;}
                
                auto is_same_block = 
                    (from >= l->v) &(from <= r->v)
                    &(to >= l->v) &(to <= r->v);
            
                if(is_same_block){
                    printf("detected as the same block : %lld <= x <= %lld\n",l->v,r->v);fflush(stdout);
                    auto is_even = bool(l->v%(sizeof(value_type)*2));
                    check_res=
                         (is_same_block& is_even)& res
                        |(is_same_block&!is_even)&!res;
                    break;
                }
            }
            
            printf("%s | ",res==check_res?"VALID" : "!!!! INVALID");
            printf("res(%d) | %lld <= {%lld %lld} <= %lld",res,l->v,from,to,r->v);
        

        
    }
    
    fclose(f_ranges);
    
    
    return 0;
}

#endif


