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

int tmain_kautil_range_exsits_interface() {
    
    using value_type = uint64_t;
    using offset_type = long;
    auto step = 10;
    auto data = std::vector<value_type>();{
        for(auto i = 0; i < 100; ++i){
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
            auto from=value_type{10},to=value_type{20};
//            from = (rand()%max+min)/step*step;
//            to = (rand()%max+min)/step*step;
            // 4 pattern
                //exact
                //contained
            auto i0 = bt.search(from,false);
            auto i1 = bt.search(to,false);
            auto is_exact = [](auto const& i0,auto const& i1)->bool{
                return
                         ((!i0.direction)&!bool(i0.nearest_pos%(sizeof(value_type)*2))) 
                        &((!i1.direction)& bool(i1.nearest_pos%(sizeof(value_type)*2)));
            };
            auto is_contained = [](auto const& in)->bool{
                return 
                    bool(in.nearest_pos%(sizeof(value_type)*2))&(in.direction<0) 
                 + !bool(in.nearest_pos%(sizeof(value_type)*2))&(in.direction>0);
            };
            auto exact = is_exact(i0,i1);
            auto contained = is_contained(i0)&is_contained(i1);
            auto size_check = (sizeof(value_type)==(-i0.nearest_pos+i1.nearest_pos));
            auto res = bool(exact+contained)&size_check; 
        
            auto i0_check_buf = value_type(0);
            auto i1_check_buf = value_type(0);
            auto i_buf_ptr = &i0_check_buf;
            pref.read_value(i0.nearest_pos,&(i_buf_ptr=&i0_check_buf));
            pref.read_value(i1.nearest_pos,&(i_buf_ptr=&i1_check_buf));
            printf("%d : (from,to){%lld %lld} ansewer{%lld,%lld} \n",res,from,to,i0_check_buf,i1_check_buf);
            int jjj = 0;
//        }
        
        
    }
    
    fclose(f_ranges);
    
    
    return 0;
}

#endif


