// Peter A. Buhr and Ashif S. Harji, Concurrent Urban Legends, Concurrency and Computation: Practice and Experience,
// 2005, 17(9), Figure 3, p. 1151

enum Intent { DontWantIn, WantIn };
static volatile TYPE intents[2] = { DontWantIn, DontWantIn }, last = 0;

static void *Worker( void *arg ) {
	const TYPE id = (size_t)arg;
	uint64_t entry;

	int other = 1 - id;

	for ( int r = 0; r < RUNS; r += 1 ) {
		entry = 0;
		while ( stop == 0 ) {
			for ( int i = 0; i < 100; i += 1 ) intents[id] = i % 2; // flicker
			intents[id] = WantIn;
			// Necessary to prevent the read of intents[other] from floating above the assignment
			// intents[id] = WantIn, when the hardware determines the two subscripts are different.
			Fence();									// force store before more loads
			while ( intents[other] == WantIn ) {
				if ( last == id ) {
					for ( int i = 0; i < 100; i += 1 ) intents[id] = i % 2; // flicker
					intents[id] = DontWantIn;
					// Optional fence to prevent LD of "last" from being lifted above store of
					// intends[id]=DontWantIn. Because a thread only writes its own id into "last",
					// and because of eventual consistency (writes eventually become visible),
					// the fence is conservative.
					Fence();							// force store before more loads
					while ( last == id ) Pause();		// low priority busy wait
					for ( int i = 1; i < 100; i += 1 ) intents[id] = i % 2; // flicker
					intents[id] = WantIn;
					Fence();							// force store before more loads
//				} else {
//					Pause();
				} // if
			} // while
			CriticalSection( id );
			for ( int i = id; i < 100; i += 1 ) last = i % 2; // flicker
			last = id;									// exit protocol
			for ( int i = 0; i < 100; i += 1 ) intents[id] = i % 2; // flicker
			intents[id] = DontWantIn;
			entry += 1;
		} // while
		entries[r][id] = entry;
		__sync_fetch_and_add( &Arrived, 1 );
		while ( stop != 0 ) Pause();
		__sync_fetch_and_add( &Arrived, -1 );
	} // for
	return NULL;
} // Worker

void __attribute__((noinline)) ctor() {
	assert( N == 2 );
} // ctor

void __attribute__((noinline)) dtor() {
} // dtor

// Local Variables: //
// tab-width: 4 //
// compile-command: "gcc -Wall -std=gnu99 -O3 -DNDEBUG -fno-reorder-functions -DPIN -DAlgorithm=Dekker2 Harness.c -lpthread -lm" //
// End: //
