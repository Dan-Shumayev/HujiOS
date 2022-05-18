////
//#include "MapReduceFramework.h"
//#include "MapReduceClient.h"
//#include "Barrier.cpp"
//
//#include <pthread.h>
//#include <math.h>
//#include <semaphore.h>
//#include <atomic>
//#include <algorithm>
//#include <iostream>
//
//using std::string;
//using std::vector;
//using std::atomic;
//
//typedef unsigned long long state_t;
//
//state_t mask32bit = pow(2, 32);
//state_t mask62bit = pow(2, 62);
//
////// ----------------- Structures -----------------
//
//typedef struct ThreadContext ThreadContext;
//
//struct JobContext {
//    int multiThreadLevel;
//    JobState *state;
//    pthread_t *threads;
//    const MapReduceClient *client;
//    const InputVec *inputPairs;
//    OutputVec *outputPairs;
//    vector<IntermediateVec> *mappedVectors;
//    vector<IntermediateVec> *shuffledVector;
//    atomic<state_t> *stateCounter;
//    Barrier *barrier;
//    sem_t *semaphore;
//    pthread_mutex_t *lock;
//    ThreadContext* threads_contexts;
//    int isWaitingForClose;
//    int totalPairsAfterMapping;
//};
//
//struct ThreadContext {
//    int threadID;
//    JobContext *jobContext;
//};
//
////// ----------------- Helper Functions -----------------
//
//state_t getNumberOfProcessedItems(state_t counter_val){
//    return counter_val & (mask32bit-1);
//}
//
//state_t incrementNextItemIdToProcess(atomic<state_t> &counter){
//    return (counter.fetch_add(mask32bit) & (mask62bit-1)) / mask32bit ;
//}
//
//stage_t getStage(state_t counter_val){
//    return static_cast<stage_t>(counter_val / mask62bit);
//}
//
//void incrementStage(atomic<state_t> &counter){
//    counter.store(getStage(counter) * mask62bit + mask62bit);
//}
//
//void addNumberOfProcessedItems(atomic<state_t> &counter, int amount){
//    counter.fetch_add(amount);
//}
//
//bool compareKeys(const IntermediatePair &lhs, const IntermediatePair &rhs) {
//    return (*lhs.first) < (*rhs.first);
//}
//
//bool areEqualK2(K2 *lhs, K2 *rhs) {
//    return !((*lhs < *rhs) || (*rhs < *lhs));
//}
//
//K2* getMaxKey(vector<IntermediateVec> *vectors){
//    K2* maxKey = nullptr;
//    for (auto& v : *vectors){
//        if ((maxKey == nullptr && !v.empty()) || (!v.empty() && (*maxKey < *(v.back().first))))
//            maxKey = v.back().first;
//    }
//    return maxKey;
//}
//
//void checkForError(int returnVal, const string& text){
//    if (returnVal == 0) return;
//    std::cerr << "system error: " << text << std::endl;
//    exit(1);
//}
//
//void map(ThreadContext *threadContext) {
//    JobContext *jobContext = threadContext->jobContext;
//    if (getStage(jobContext->stateCounter->load()) == UNDEFINED_STAGE){
//        incrementStage(*jobContext->stateCounter);
//    }
//    atomic<state_t> *counter = jobContext->stateCounter;
//    unsigned long totalPairsToMap = jobContext->inputPairs->size();
//    unsigned long currPairId;
//
//    while ((currPairId = incrementNextItemIdToProcess(*counter)) < totalPairsToMap){
//
//        InputPair pair = jobContext->inputPairs->at(currPairId);
//        jobContext->client->map(pair.first, pair.second, threadContext);
//        addNumberOfProcessedItems(*counter, 1);
//    }
//
//}
//
//void sort(ThreadContext *threadContext) {
//    JobContext *jobContext = threadContext->jobContext;
//    IntermediateVec &v = (*jobContext->mappedVectors)[threadContext->threadID];
//
//    if (!v.empty()) {
//        std::sort(v.begin(), v.end(), compareKeys);
//    }
//}
//
//void shuffle(ThreadContext *threadContext) {
//    atomic<state_t> *counter = threadContext->jobContext->stateCounter; // For increment num of processed
//
//    vector<IntermediateVec> *shuffledVectors = threadContext->jobContext->shuffledVector; // Shuffled ones
//    vector<IntermediateVec> *vectors = threadContext->jobContext->mappedVectors; // All the inter vecs
//    K2* maxKey; // Point to the current max key
//
//    while ((maxKey = getMaxKey(vectors)) != nullptr) // Go over all the keys
//    {
//        auto newVec = IntermediateVec(); // Current inter vec to be pushed to shuffled
//                                            // - with current maximum same key
//
//        for (auto& v : *vectors) // Iterate over each thread's inter-vec
//        {
//            while (!v.empty() && areEqualK2((v.back().first), maxKey)) // This inter-vec contains this key?
//            {
//                newVec.push_back(v.back()); // Then push to the current shuffled inter-vec
//                v.pop_back();
//
//                addNumberOfProcessedItems(*counter, 1); // Increment num of processed
//            }
//        }
//        shuffledVectors->push_back(newVec); // Push it to our shuffled queue
//    }
//}
//
//void reduce(ThreadContext *threadContext) {
//    JobContext *jobContext = threadContext->jobContext;
//    atomic<state_t> *counter = jobContext->stateCounter;
//
//    unsigned long totalVecToReduce = jobContext->shuffledVector->size();
//    unsigned long currVecId;
//    while ((currVecId = incrementNextItemIdToProcess(*counter)) < totalVecToReduce){
//        IntermediateVec vec = jobContext->shuffledVector->at(currVecId);
//        jobContext->client->reduce(&vec, threadContext);
//        addNumberOfProcessedItems(*counter, vec.size());
//    }
//}
//
//void* workerFlow(void *threadContext){
//    auto context = (ThreadContext*) threadContext;
//    map(context);
//    sort(context);
//    context->jobContext->barrier->barrier();
//    sem_wait(context->jobContext->semaphore);
//    sem_post(context->jobContext->semaphore);
//    reduce(context);
//    return nullptr;
//}
//
//
////// ----------------- Framework Functions -----------------
//
//void* mainFlow(void* threadContext){
//    auto* tc = (ThreadContext*) threadContext;
//
//    // map & sort of thread 0
//
//    map(tc);
//    sort(tc);
//
//    tc->jobContext->barrier->barrier();
//
//    int totalItems = 0;
//    for (auto& v: *tc->jobContext->mappedVectors){
//        totalItems += v.size();
//    }
//    tc->jobContext->totalPairsAfterMapping = totalItems;
//
//    // shuffle
//
//    incrementStage(*tc->jobContext->stateCounter);
//
//    shuffle(tc);
//
//    incrementStage(*tc->jobContext->stateCounter);
//
//    sem_post(tc->jobContext->semaphore);
//
//    // reduce
//    reduce(tc);
//
//    // wait for threads to terminate
//    int res;
//    for (int i = 1; i < tc->jobContext->multiThreadLevel; i++) {
//        res = pthread_join(tc->jobContext->threads[i], nullptr);
//        checkForError(res, "pthread_join");
//    }
//    return nullptr;
//}
//
//JobHandle startMapReduceJob(const MapReduceClient& client,
//                            const InputVec& inputVec,
//                            OutputVec& outputVec,
//                            int multiThreadLevel){
//    // init variables
//    auto* threads = new pthread_t[multiThreadLevel];
//    auto* threadContexts = new ThreadContext[multiThreadLevel];
//    auto* counter = new atomic<state_t>(0);
//    auto* mappedVectors = new vector<IntermediateVec>(multiThreadLevel, IntermediateVec());
//    auto* shuffledVector = new vector<IntermediateVec>(0);
//    auto* state = new JobState {UNDEFINED_STAGE, 0};
//
//    int res;
//    // init barrier & mutex
//    auto* barrier = new Barrier(multiThreadLevel);
//    auto* lock = new pthread_mutex_t();
//    res = pthread_mutex_init(lock, nullptr);
//    checkForError(res, "pthread_mutex_init");
//
//    // init semaphore
//    sem_t* sem = new sem_t();
//    res = sem_init(sem, 0, 0);
//    checkForError(res, "sem_init");
//
//    auto *jobContext = new JobContext{multiThreadLevel, state, threads, &client, &inputVec, &outputVec,
//                                      mappedVectors, shuffledVector, counter, barrier, sem, lock, threadContexts,false, 0};
//
//    // create worker thread 0
//    threadContexts[0] = {0, jobContext};
//    res = pthread_create(threads, nullptr, mainFlow, threadContexts);
//    checkForError(res, "pthread_create");
//
//    // create worker threads
//    for (int i = 1; i < multiThreadLevel; ++i){
//        threadContexts[i] = {i, jobContext};
//        res = pthread_create(threads + i, nullptr, workerFlow, &threadContexts[i]);
//        checkForError(res, "pthread_create");
//    }
//
//    return static_cast<void*>(jobContext);
//}
//
//void getJobState(JobHandle job, JobState* state){
//
//    auto *jobContext = (JobContext*) job;
//
//    state_t counter = jobContext->stateCounter->load(); // atomicCurrStage.load()
//    float processedItems = getNumberOfProcessedItems(counter); // atomicCurrStage.load() & (pow(2, 32)-1)
//    state->stage = getStage(counter); // static_cast<stage_t>(atomicCurrStage.load() / pow(2, 62))
//
//    // atomic<state_t> *stateCounter;
//    // getNumberOfProcessedItems(counter)  ->   counter_val & (mask32bit-1);
//    // getStage(counter_val) ->    static_cast<stage_t>(counter_val / mask62bit);
//    // state_t mask32bit = pow(2, 32);
//    // state_t mask62bit = pow(2, 62);
//
//    float totalItems;
//    switch (state->stage){
//        case MAP_STAGE:{
//            totalItems = jobContext->inputPairs->size();
//            break;
//        }
//        case SHUFFLE_STAGE:
//        case REDUCE_STAGE:{
//            totalItems = jobContext->totalPairsAfterMapping;
//            break;
//        }
//        case UNDEFINED_STAGE:{
//            processedItems = 0;
//            totalItems = 1;
//            break;
//        }
//    }
//
//    totalItems = totalItems != 0 ? totalItems : 1;
//
//    state->percentage = (processedItems / totalItems) * 100;
//}
//
//void waitForJob(JobHandle job){
//    auto *jobContext = (JobContext*) job;
//    getJobState(job, jobContext->state);
//    if (!jobContext->isWaitingForClose){
//        int res = pthread_join(jobContext->threads[0], nullptr);
//        checkForError(res, "pthread_join");
//        jobContext->isWaitingForClose = true;
//    }
//}
//
//void closeJobHandle(JobHandle job){
//    waitForJob(job);
//    auto *jobContext = (JobContext*) job;
//    sem_destroy(jobContext->semaphore);
//    pthread_mutex_destroy(jobContext->lock);
//
//    delete jobContext->mappedVectors;
//    delete jobContext->shuffledVector;
//    delete jobContext->barrier;
//    delete[] jobContext->threads;
//    delete jobContext->semaphore;
//    delete jobContext->lock;
//    delete jobContext->stateCounter;
//    delete jobContext->state;
//    delete[] jobContext->threads_contexts;
//    delete jobContext;
//}
//
//void emit2 (K2* key, V2* value, void* context){
//    auto* threadContext = (ThreadContext*) context;
//    JobContext *jobContext = threadContext->jobContext;
//    auto newPair = std::pair<K2*, V2*>(key, value);
//    pthread_mutex_lock(jobContext->lock);
//    (*jobContext->mappedVectors)[threadContext->threadID].push_back(newPair);
//    pthread_mutex_unlock(jobContext->lock);
//}
//
//void emit3 (K3* key, V3* value, void* context){
//    auto* threadContext = (ThreadContext*) context;
//    JobContext *jobContext = threadContext->jobContext;
//    auto newPair = std::pair<K3*, V3*>(key, value);
//    pthread_mutex_lock(jobContext->lock);
//    jobContext->outputPairs->push_back(newPair);
//    pthread_mutex_unlock(jobContext->lock);
//}