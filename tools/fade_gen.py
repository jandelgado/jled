from fade_funcs import f8

def gen_table(values):
    print("static constexpr uint8_t kFadeOnTable[] = {", end="")
    print(*values, sep=",", end="")
    print("};")


num = 16
period = 256
values = [f8(t,period) for t in range(0, period, int(period/num))]
values = values + [65535]
gen_table(values)
print(f"num entries {len(values)}")

