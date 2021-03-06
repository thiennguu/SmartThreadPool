/****************************************************************************\
 * Created on Mon Jul 30 2018
 * 
 * The MIT License (MIT)
 * Copyright (c) 2018 leosocy
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the ",Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED ",AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
\*****************************************************************************/

#include "smart_thread_pool.h"
#include <iostream>

int main(int argc, char** argv) {

  // ********************How to init `SmartThreadPool`********************
  //
  // using stp::SmartThreadPool;
  // using stp::SmartThreadPoolBuilder;
  // SmartThreadPoolBuilder builder;

  // ********Build by calling a chain.********
  // builder.AddClassifyPool(const char* pool_name,
  //                         uint8_t capacity,
  //                         uint8_t init_size);
  // ******** Such as:
  // builder.AddClassifyPool("DefaultPool", 16, 4)
  //        .AddClassifyPool("CPUBoundPool", 8, 4)
  //        .AddClassifyPool("IOBoundPool", 16, 8)
  // auto pool = builder.BuildAndInit();  // will block current thread
  //
  // ***********************************************************************

  // ******************************How to join a task******************************
  //
  // pool->ApplyAsync(function, args...);
  // ******** Such as:
  // 1. Run a return careless task.
  // pool->ApplyAsync("IOBoundPool", TaskPriority::MEDIUM, [](){ //DoSomeThing(args...); }, arg1, arg2, ...);
  //
  // 2. Run a return careful task.
  // auto res = pool->ApplyAsync("CPUBoundPool", TaskPriority::HIGH, [](int count){ return count; }, 666);
  // auto value = res.get();
  //
  // or you can set a timeout duration to wait for the result to become available.
  //
  // std::future_status status = res.wait_for(std::chrono::seconds(1)); // wait for 1 second.
  // if (status == std::future_status::ready) {
  //   std::cout << "Result is: " << res.get() << std::endl;
  // } else {
  //   std::cout << "Timeout" << std::endl;
  // }
  //
  // *******************************************************************************

  using stp::SmartThreadPoolBuilder;
  using stp::TaskPriority;
  SmartThreadPoolBuilder builder;
  builder.AddClassifyPool("DefaultPool", 8, 4)
         .AddClassifyPool("CPUBoundPool", 8, 4)
         .AddClassifyPool("IOBoundPool", 64, 32)
         .EnableMonitor(std::chrono::seconds(5));
  auto pool = builder.BuildAndInit();
  
  for (int i = 0; i < 64; ++i) {
    for (unsigned char i = 0; i < 5; ++i) {
      pool->ApplyAsync("IOBoundPool", static_cast<TaskPriority>(i), [](unsigned char i) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        printf("%d\n", i);
      }, i);
    }
  }
  pool->ApplyAsync("IOBoundPool", TaskPriority::HIGH, [](){
    int repeat_times = 5;
    while(--repeat_times >= 0) {
      printf("IOBoundPool Task\n");std::this_thread::sleep_for(std::chrono::seconds(2));
    }
  });
  auto res = pool->ApplyAsync("CPUBoundPool", TaskPriority::MEDIUM, [](int x, int y){
    return x + y;
  }, 1, 2);
  auto value = res.get();
  printf("added result: %d\n", value);

  pool->StartAllWorkers();
}