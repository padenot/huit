env = Environment()
env.Append(CCFLAGS = ['-g','-Wall','-pedantic'])
env.Append(LIBS = ['sndfile'])

env.Program('huit', source=Glob('src/*.cpp'))

