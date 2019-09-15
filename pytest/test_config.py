from RLTest import Env
        
def testConfigHelp(env):
    env.expect('EXAMPLE.CONFIG', 'HELP').equal([['LONG', 'LONG help msg', 'configurable at runtime : yes'], 
                                                ['DOUBLE', 'DOUBLE help msg', 'configurable at runtime : yes'], 
                                                ['BOOL', 'BOOL help msg', 'configurable at runtime : yes'], 
                                                ['CSTR', 'CSTR help msg', 'configurable at runtime : yes'], 
                                                ['REDISSTR', 'REDISSTR help msg', 'configurable at runtime : yes'], 
                                                ['LONG_NOT_CONFIGURABLE', 'not configurable long value', 'configurable at runtime : no'], 
                                                ['CALLBACK', 'demonstrate how to use callback with config values', 'configurable at runtime : yes']])
    
def testConfigWrongArity(env):
    env.expect('EXAMPLE.CONFIG', 'GET').error()
    env.expect('EXAMPLE.CONFIG', 'GET', 'wrong', 'wrong2').error()
    env.expect('EXAMPLE.CONFIG', 'SET').error()
    env.expect('EXAMPLE.CONFIG', 'SET', 'wrong1', 'wrong2', 'wrong3').error()
    
def testConfigWithUnknowsValue(env):
    env.expect('EXAMPLE.CONFIG', 'GET', 'wrong1').error()
    env.expect('EXAMPLE.CONFIG', 'SET', 'wrong1', 'wrong2').error()
    
def testConfigDefaultVals(env):
    env.expect('EXAMPLE.CONFIG', 'GET', 'LONG').equal(1)
    env.expect('EXAMPLE.CONFIG', 'GET', 'DOUBLE').equal('1')
    env.expect('EXAMPLE.CONFIG', 'GET', 'BOOL').equal('disabled')
    env.expect('EXAMPLE.CONFIG', 'GET', 'CSTR').equal('defaultVal')
    env.expect('EXAMPLE.CONFIG', 'GET', 'REDISSTR').equal('defaultRedisVal')
    env.expect('EXAMPLE.CONFIG', 'GET', 'LONG_NOT_CONFIGURABLE').equal(1)
    
def testConfigSetVals(env):
    env.expect('EXAMPLE.CONFIG', 'SET', 'LONG', '2').equal('OK')
    env.expect('EXAMPLE.CONFIG', 'GET', 'LONG').equal(2)
    
    env.expect('EXAMPLE.CONFIG', 'SET', 'DOUBLE', '2').equal('OK')
    env.expect('EXAMPLE.CONFIG', 'GET', 'DOUBLE').equal('2')
    
    env.expect('EXAMPLE.CONFIG', 'SET', 'BOOL').equal('OK')
    env.expect('EXAMPLE.CONFIG', 'GET', 'BOOL').equal('enabled')
    
    env.expect('EXAMPLE.CONFIG', 'SET', 'CSTR', 'newVal').equal('OK')
    env.expect('EXAMPLE.CONFIG', 'GET', 'CSTR').equal('newVal')
    
    env.expect('EXAMPLE.CONFIG', 'SET', 'REDISSTR', 'newRedisVal').equal('OK')
    env.expect('EXAMPLE.CONFIG', 'GET', 'REDISSTR').equal('newRedisVal')
    
def testConfigSetUnconfigurableVal(env):
    env.expect('EXAMPLE.CONFIG', 'SET', 'LONG_NOT_CONFIGURABLE', '2').error()
    
def testConfigSetWithmoduleArgs():
    args = 'LONG 2 DOUBLE 2 BOOL CSTR newVal REDISSTR newRedisVal LONG_NOT_CONFIGURABLE 2 CALLBACK VAL'
    env = Env(moduleArgs=args)
    
    env.expect('EXAMPLE.CONFIG', 'GET', 'LONG').equal(2)
    env.expect('EXAMPLE.CONFIG', 'GET', 'DOUBLE').equal('2')
    env.expect('EXAMPLE.CONFIG', 'GET', 'BOOL').equal('enabled')
    env.expect('EXAMPLE.CONFIG', 'GET', 'CSTR').equal('newVal')
    env.expect('EXAMPLE.CONFIG', 'GET', 'REDISSTR').equal('newRedisVal')
    env.expect('EXAMPLE.CONFIG', 'GET', 'LONG_NOT_CONFIGURABLE').equal(2)
    
def testConfigWithCallback(env):
    env.expect('EXAMPLE.CONFIG', 'GET', 'CALLBACK').equal('OK')
    env.expect('EXAMPLE.CONFIG', 'SET', 'CALLBACK', 'VAL').equal('OK')
