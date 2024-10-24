from ns import ns
print(dir(ns.CommandLine))

cmd = ns.CommandLine()
int_arg = 0
cmd.AddValue("intArg", "an int argument", int_arg)