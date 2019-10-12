# Ghost_Diary

> Try writing in this [ghost diary](ghostdiary). It's also found in `/problems/ghost-diary_4_e628b10cf58ea41692460c7ea1e05578` on the shell server.

Even though our team's original solution used a technique with overlapping chunks, this write-up will feature a separate solution utilizing an untried idea suggested before the problem was solved. As the idea was independently generated, motivation is provided.

In one sentence, this exploit uses a poison-null overflow to trigger a back-consolidation in order to put an already-free chunk onto a freelist twice.

We are given a stripped 64-bit binary called `ghostdiary` as well as a directory on the shell server. Checking the libc version on the server gives us version `2.27` (which is the standard version for Ubuntu systems as of writing). Let's also do a `checksec`:
```
[*] '/.../ghostdiary'
    Arch:     amd64-64-little
    RELRO:    Full RELRO
    Stack:    Canary found
    NX:       NX enabled
    PIE:      PIE enabled

```
It looks like everything is enabled, so we have quite the task ahead of us. After running, we are given a menu with the following options: `New page`, `Talk with ghost`, `Listen to ghost`, `Burn the page`, and `Go to sleep`. We suspect that these operations correspond to a fairly standard heap problem with `create`, `edit`, `read`, `delete`, and `exit`. We, as always, then begin attacking the problem by reversing the binary. The full pseudocode can be found [here](ghost_diary.cc).

Running the standard checks on stupid exploits is rather unfruitful. The ``delete_page`` method correctly sets the pointer to ``NULL`` after freeing, so any use-after-free exploit will have to work nontrivially.

So what do we have? After sufficient staring, the key bug can be found in the ``edit_page_input`` method:
```c
void edit_page_input(struct page p) {
	unsigned int i = 0;
	if (p.size != 0) {
		for (; i != p.size; i++)
...
		p.content[i] = '\x00'; // OFF BY ONE BUG
	}
}
```
Indeed, the program does no checks on the location of where it places the last null byte it uses to terminate the inputted string into the page. I.e., this null byte could overflow past the data of the chunk, and because of the organization of chunks in the heap, this could in theory overflow the size and prev-in-use metadata in the next chunk. Let's keep this in mind.

We are using libc 2.27, and tcache in libc 2.27 is even more dangerous than usual, so philosophically, it would be nice to get a double-free with one of the frees on tcache, as this will give us [the most arbitrary of writes](https://github.com/shellphish/how2heap/blob/master/glibc_2.26/tcache_poisoning.c). Notably, having a double-free with a free on tcache will let us coerce tcache into handing malloc an arbitrary pointer and letting us write there using typical double-free techniques.

However, the program also gives us 20 mallocs before the program complains, which is enough to overcome the number of chunks allowed in one size of a tcache linked list. If the only solution only placed chunks onto the tcache, then there would likely be a limitation to 6 mallocs as seen in zero_to_hero. But conversely, if the only solution purely used the old non-tcache free, then we probably would have been handed an older version of libc, as in sice_cream. So we are probably going to end up using both the free onto tcache and free onto one of the various older freed linked lists.

Alright, the only tool we currently have is that null-byte overflow, and altering the size of a chunk generally does not matter much to tcache or the malloc freed linked lists; recall after a pointer is freed once, it gets removed, so alterations of size will not allow us to get that pointer back to free it. This leaves us with corrupting the prev-in-use bit. The following would be the set up.
```
[chunk 1]: 0x0000000000000000 0x0000000000000121 <- size = 0x121, prev-in-use bit set
           0x0000000000000000 0x0000000000000000
           ...
[chunk 2]: 0x0000000000000000 0x0000000000000101 <- size = 0x100, prev-in-use bit set
           ...
```
Then so far our unfinished exploit looks would have us overflow to write ``0x00`` into the metadata, as such.
```
[chunk 1]: 0x0000000000000000 0x0000000000000121 <- size = 0x121, prev-in-use bit set
           0xdeadbeefdeadbeef 0xdeadbeefdeadbeef
           ...
[chunk 2]: 0xdeadbeefdeadbeef 0x0000000000000100 <- size = 0x100, prev-in-use bit not set!
           ...
```
Now, tcache does not care about the prev-in-use bit (it only cares about size for which freed linked list to place in), so if we want to abuse this corruption, we have to use the older freed linked lists. However, with the prev-in-use bit unset, the prev-size portion of ``chunk 2`` will now be checked in order for ``free`` to figure out where the previous chunk is, so we change the setup to the following.
```
[chunk 1]: 0x0000000000000000 0x0000000000000121 <- size = 0x121, prev-in-use bit set
           0xdeadbeefdeadbeef 0xdeadbeefdeadbeef
           ...
[chunk 2]: 0x0000000000000120 0x0000000000000100 <- size = 0x100, prev-in-use bit not set!
           ...
```
Ok, so after overcoming tcache, what does freeing ``chunk 2`` look like? Here is the corresponding code from malloc.c's ``free``.
```c
    /* consolidate backward */
    if (!prev_inuse(p)) {
      prevsize = prev_size (p);
      size += prevsize;
      p = chunk_at_offset(p, -((long) prevsize));
      if (__glibc_unlikely (chunksize(p) != prevsize))
        malloc_printerr ("corrupted size vs. prev_size while consolidating");
      unlink_chunk (av, p);
    }
    ...
```
The chunk corresponding to ``p``, which at the beginning would be``chunk 2`` but is later set to ``chunk 1``, is then thrown onto the unsorted chunk freed linked list. Observe that this is a bit dangerous: This backwards consolidation does  no checks on whether the previous chunk has already been freed and instead simply calls ``unlink`` and throws it onto the unsorted freelist, with no further questions asked. This is exploitable. Indeed, if we were to place the previous chunk onto tcache and then somehow coerce the ``unlink`` macro into doing the rough equivalent of nothing, this chunk gets placed onto both freed linked lists simultaneously with minimal checks. This grants us our double-free and so our arbitrary write.

As the finishing sidenote, we do have to think a bit to overcome the ``unlink`` macro because of its ``P->fd->bk == P`` checks, especially because we want the ``chunk 1`` to already be freed onto the tcache freed linked list, and tcache does place a pointer at ``P->fd``. To fix this, we add another chunk before ``chunk 1`` to complete the set-up.
```
[chunk 0]: 0x0000000000000000 0x0000000000000121
           0x0000000000000000 0x0000000000000000
	   0x[ptr to chunk 1] 0x[ptr to chunk 1] <- we set both chunk 0 ptr->fd and ->bk to chunk 1
	   ...
[chunk 1]: 0x0000000000000000 0x0000000000000121
           0x[ptr to chunk 0] 0x[ptr to chunk 0] <- we set both fd and bk to point to chunk 0
           ...
[chunk 2]: 0x0000000000000120 0x0000000000000100
           ...
```
Under this configuration, freeing ``chunk 0`` and then ``chunk 1`` onto tcache will add a pointer to ``chunk 0`` where there already is one and so not immediately change the configuration. However, the resulting ``unlink`` call will check if ``chunk 1 -> ptr to chunk 0 -> pointer to chunk 1 = chunk 1``, and of course we can check that it does. This completes the key ideas of the exploit.

Breifly, the above requires a heap leak. This is not hard to come by because, as we stated above, tcache stores heap addresses in its freed linked list, so we may simply request one of these chunks back and read in the address.

From here the rest of the exploit is details. The usual finish from arbitrary write is to write into, say, ``__free_hook`` to a ``one_gadget`` in order to get a shell. Both finding the location in code of both of these pointers will require a libc leak, but this is relatively easy to come by because any chunk placed into the unsorted chunk freed linked list (such as the one described above) will be part of a doubly-linked list whose head is stored in libc. Thus, when we ask for chunk back from the unsorted chunk freed linked list, w can simply read off the libc address still stored in its contents. In this way we can write directly to ``__free_hook`` to call our ``one_gadget`` and get our shell.

The full code, with comments added, can be found in [exploit.py](exploit.py). And as fruits for our labors, we bring the flag home:
> ``picoCTF{nu11_byt3_Gh05T_82783d57}``
