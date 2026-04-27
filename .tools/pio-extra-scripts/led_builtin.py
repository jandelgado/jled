Import("env")

if env["BOARD"] == "esp32dev":
    env.Append(CPPDEFINES=[("LED_BUILTIN", "2")])
