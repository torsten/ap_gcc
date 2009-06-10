@g_gdb_stream = nil

def get_symbol_name_for_address addr
  @g_gdb_stream.puts "info symbol #{addr}"
  symbol_info = @g_gdb_stream.readline
  symbol_info.split(" ")[0]
end

def get_fileinfo_for_symbol symbol
  @g_gdb_stream.puts "info line #{symbol}"
  symbol_info = @g_gdb_stream.readline.split(" ")
  [symbol_info[3].delete('"'), symbol_info[1]]
end

def get_address_for_symbol_name symbol
  # TODO implement me
  # info address <sym-name>
end

def get_class_info symbol
  @g_gdb_stream.puts "ptype #{symbol}"

  # TODO: implement me
  # ptype klass
  # type = class klass {
  #   public:
  #     klass(void);
  #     ~klass(int);
  #     void foo();
  #     void foo(int);
  # }
end

IO.popen("gdb -x ./igdb_starter.gdbscript -q ./profile_test", "w+") { |f|

  # store the stream globally
  @g_gdb_stream = f

  # read the reading symbols line
  f.readline

  addr = '0x285c'
  name = get_symbol_name_for_address(addr)
  info = get_fileinfo_for_symbol(name)

  puts "symbol #{addr} is called #{name}, coded in file #{info[0]} at line #{info[1]}"
}
