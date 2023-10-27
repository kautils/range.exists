#ifdef TMAIN_KAUTIL_RANGE_EXSITS_INTERFACE

int tmain_kautil_range_exsits_interface();
int main(){ return tmain_kautil_range_exsits_interface(); }


#include "exists.hpp"
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





#include <set>
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
    
        srand((uintptr_t)&pref);
        
            auto diff = 1;
//            point
//            auto from=value_type{0},to=value_type{0}; // expect false
//            auto from=value_type{8},to=value_type{11}; // expect false
//            auto from=value_type{1001},to=value_type{1001}; // expect false
//            auto from=value_type{9},to=value_type{11}; // expect false
//            auto from=value_type{991},to=value_type{991}; // expect true
//            auto from=value_type{999},to=value_type{999}; // expect true
//            auto from=value_type{1000},to=value_type{1000}; // expect true
//            auto from=value_type{15},to=value_type{15}; // expect true
        

//            one block 
//            auto from = value_type{0},to = value_type{8}; //expect false
//            auto from = value_type{0},to = value_type{10}; //expect false
//            auto from = value_type{0},to = value_type{11}; //expect false
//            auto from = value_type{0},to = value_type{15}; //expect false
//            auto from = value_type{0},to = value_type{19}; //expect false
//            auto from = value_type{0},to = value_type{20}; //expect false
            auto from = value_type{10},to = value_type{20};  //expect true
//            auto from = value_type{11},to = value_type{15};  //expect true
//            auto from = value_type{9},to = value_type{15};  //expect false
//            auto from = value_type{9},to = value_type{21};  //expect false
                
//            two block
//            auto from=value_type{0},to=value_type{30}; // expect false
//            auto from=value_type{5},to=value_type{30}; // expect false
//            auto from=value_type{9},to=value_type{30}; // expect false
//            auto from=value_type{9},to=value_type{45}; // expect false
//            auto from=value_type{25},to=value_type{65}; // expect false
        
//            three block
//            auto from=value_type{0},to=value_type{50}; // expect false
//            auto from=value_type{5},to=value_type{50}; // expect false
//            auto from=value_type{9},to=value_type{50}; // expect false
//            auto from=value_type{19},to=value_type{60}; // expect false
//            auto from=value_type{19},to=value_type{65}; // expect false
//            auto from=value_type{24},to=value_type{65}; // expect false
//            
//           overflow 
//            auto from=value_type{0},to=value_type{5}; // expect false
//            auto from=value_type{1000},to=value_type{1010}; // expect false

            auto ext = kautil::range::exists{&pref};
            auto res = ext.exec(from,to);
        
            
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
                l = &*cur;
                if(++cur==e){ break;}
                r = &*cur;
                
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