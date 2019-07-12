# perflab
## Q1
       The trend that is shown is that the average clocks per element appended 
       is rather high for small collection sizes, but it evens out around 10 
       average clocks per element appended at around a total collection size of 
       1500. This strange decrease from small sizes to large sizes can be 
       described through the performance of smaller lists. In the arraylist.c 
       file, we see that appending calls the ensure_space_available function, 
       which doubles the space allocated for storage if not available. With 
       small arrays, this function will have to make more space, while larger 
       arrays will already have this space allocated. It is more likely to need 
       more space for the small arrays, since this function just mutliplies the
       capacity by two.
## Q3
       There is a pretty significant improvement. The original graph is now 
       shifted down about 4 clocks per element appended. This should improve the
       original method because realloc checks to see if the additional memory is
       available right next to where the current memory is. This can this just 
       be appended then. The original method just mallocs, which then has to 
       move the original list, which might not be necessary. Again, the graph 
       suggest constant time at larger scales, because we use the original
       method of multiplying the space by two.
## Q5
       Again, there is a very large improvement. I would say the new method is 
       about 4x better than the manually shifting elements. The memory utility
       functions improve the performance, because it is a single removal and 
       placement of the memory. This minimizes the RAM accesses, which has to be
       done each time in the original looping strategy. Therefore, the new 
       method is still O(N), but the factor is a lot less. The new strategy 
       makes full use of the alignment to copy word per word, while the loop
       goes byte per byte. This significantly decreases the time needed.
## Q8
       The differences between the malloc and small pool versions is that the 
       malloc version must allocate memory for a node each time, while the small
       pool already has the small pool needed. Therefore, the small pool version
       will be faster based on that time needed to allocate the memory. This is 
       why both the insert and append are just a shifted down version of the 
       malloc version.
## Q9
       The obvious trend is that the time needed to iterate over the linked list
       increases as the list increases. We have to loop over each node, so this 
       makes sense. However,there is a very small increase, almost flat, from 
       sizes 200000 to 400000. After that, there is a dramatic increase that 
       continues for the rest of the time. This could be a result of nodes 
       pointing to random places in memory and having larger numbers of nodes
       could finally hit a point where it becomes worse. This could also be a 
       result of finding the breaking point of the local cache.
## Q10
       If most arrays are like our optimized one and most linked lists are like 
       the initial one, we should honestly use array lists for a majority of our
       work. For iterating, the array lists are always better because we can 
       just index. For inserting, the optimized array list is better until 
       around a collection size of 2500. For appending, the initial linked list 
       is the worst. In most cases, the array list is obviously better and 
       should be used. Honestly, the only main advantage of linked lists is 
       easily being able to add nodes to the front of the list, since we can 
       modify the head. In most practice, people use linked lists with the 
       intention of doing this.

# Extra Credit
## EC1
       This does not seem to make a difference. Although there is some noise, 
       the -O3 optimization level is the same as the -O2 optimization level of 
       time.
## EC2
       The appending function calls ensure_space_available, which doubles the 
       memory when necessary. Therefore, we see these spikes when the memory has
       to be doubled, which takes time to complete. At all the other spots, 
       there is enough space, so nothing has to be done.
## EC3
       Didn't do :(
