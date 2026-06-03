const SCHEMA_RAW = {
  "registers": {
    "ROOT": {
      "VAR": {
        "type": "object",
        "name": "ScriptVariable",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "Number",
            "name": "value",
            "allowedTypes": {
              "bool": false,
              "float": true,
              "int": true,
              "uint": true
            }
          }
        ]
      },
      "CONSTVAR": {
        "type": "object",
        "name": "ScriptVariableReadOnly",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "Number",
            "name": "value",
            "allowedTypes": {
              "bool": false,
              "float": true,
              "int": true,
              "uint": true
            }
          }
        ]
      },
      "WRITEVAR": {
        "type": "object",
        "name": "ScriptVariableWriteOnlyTest",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          }
        ]
      },
      "ARRAY": {
        "type": "object",
        "name": "ScriptArray",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "Bool",
            "name": "readonly",
            "default": false
          },
          {
            "type": "ArrayOfPrimitives",
            "name": "items",
            "required": true,
            "primitiveTypeFlags": "07"
          }
        ]
      },
      "DIN": {
        "type": "object",
        "name": "DigitalInput",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "HardwarePin",
            "name": "pin",
            "required": true,
            "mode": "IN"
          }
        ]
      },
      "DOUT": {
        "type": "object",
        "name": "DigitalOutput",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "HardwarePin",
            "name": "pin",
            "required": true,
            "mode": "OUT"
          }
        ]
      },
      "DPOUT": {
        "type": "object",
        "name": "SinglePulseOutput",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "HardwarePin",
            "name": "pin",
            "required": true,
            "mode": "IN"
          },
          {
            "type": "UInt",
            "name": "plength",
            "default": 500,
            "minValue": 1,
            "maxValue": 0
          },
          {
            "type": "UInt",
            "name": "activeLevel",
            "default": 0,
            "minValue": 0,
            "maxValue": 1
          }
        ]
      },
      "ADC": {
        "type": "object",
        "name": "AnalogInput",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "HardwarePin",
            "name": "pin",
            "required": true,
            "mode": "AIN"
          }
        ]
      },
      "1WTG": {
        "type": "object",
        "name": "OneWireTempGroup",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "_byref_",
            "name": "refreshtime"
          },
          {
            "type": "ArrayOfObjects",
            "name": "items",
            "required": true,
            "subtype": "OneWireTempBus"
          }
        ]
      },
      "1WTB": {
        "type": "object",
        "name": "OneWireTempBusAtRoot",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "_byref_",
            "name": "refreshtime"
          },
          {
            "type": "HardwarePin",
            "name": "pin",
            "required": true,
            "mode": "OUT|IN"
          },
          {
            "type": "ArrayOfObjects",
            "name": "items",
            "required": true,
            "subtype": "OneWireTempDevice"
          }
        ]
      },
      "1WTD": {
        "type": "object",
        "name": "OneWireTempDeviceAtRoot",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "_byref_",
            "name": "refreshtime"
          },
          {
            "type": "HardwarePin",
            "name": "pin",
            "required": true,
            "mode": "OUT|IN"
          },
          {
            "type": "StringHexBytes",
            "name": "romid",
            "required": true,
            "default": "00:00:00:00:00:00:00:00",
            "byteCount": 8
          }
        ]
      },
      "DHT": {
        "type": "object",
        "name": "DHT",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Error",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "StringAnyOfByFuncConstrained",
            "name": "model",
            "required": true,
            "default": "DHT11",
            "allowedValues": [
              "DHT11",
              "DHT22",
              "AM2302",
              "RTH03"
            ]
          },
          {
            "type": "_byref_",
            "name": "refreshtime"
          },
          {
            "type": "HardwarePin",
            "name": "pin",
            "required": true,
            "mode": "OUT|IN"
          }
        ]
      },
      "TX433": {
        "type": "object",
        "name": "TX433",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "HardwarePin",
            "name": "pin",
            "required": true,
            "mode": "OUT"
          },
          {
            "type": "ArrayOfRegistryItems",
            "name": "units",
            "required": true,
            "regPath": "ROOT.TX433"
          }
        ]
      },
      "REGO600": {
        "type": "object",
        "name": "REGO600",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "HardwarePin",
            "name": "rxpin",
            "required": true,
            "mode": "IN"
          },
          {
            "type": "HardwarePin",
            "name": "txpin",
            "required": true,
            "mode": "OUT"
          },
          {
            "type": "UInt",
            "name": "requestDelayMs",
            "default": 10,
            "minValue": 0,
            "maxValue": 0
          },
          {
            "type": "_byref_",
            "name": "refreshtime"
          },
          {
            "type": "ArrayOfObjects",
            "name": "items",
            "required": true,
            "gui": [
              "RenderAllAllowedValues",
              "UseInline"
            ],
            "subtype": "REGO600_Register",
            "renderAllAllowedValuesFromStringConstraint": "regname"
          }
        ]
      },
      "I2C": {
        "type": "object",
        "name": "I2C_Master",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "HardwarePin",
            "name": "sckpin",
            "required": true,
            "mode": "OUT"
          },
          {
            "type": "HardwarePin",
            "name": "sdapin",
            "required": true,
            "mode": "OUT|IN"
          },
          {
            "type": "ArrayOfRegistryItems",
            "name": "items",
            "required": true,
            "regPath": "ROOT.I2C_Master"
          }
        ]
      },
      "WS2812": {
        "type": "object",
        "name": "WS2812",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "HardwarePin",
            "name": "pin",
            "required": true,
            "mode": "OUT"
          },
          {
            "type": "UInt",
            "name": "ledcount",
            "required": true,
            "default": 1,
            "minValue": 1,
            "maxValue": 0
          },
          {
            "type": "StringAnyOfByFuncConstrained",
            "name": "format",
            "required": true,
            "default": "RGB",
            "allowedValues": [
              "RGB",
              "RBG",
              "GRB",
              "GBR",
              "BRG",
              "BGR",
              "WRGB",
              "WRBG",
              "WGRB",
              "WGBR",
              "WBRG",
              "WBGR",
              "RWGB",
              "RWBG",
              "GWRB",
              "GWBR",
              "BWRG",
              "BWGR",
              "RGWB",
              "RBWG",
              "GRWB",
              "GBWR",
              "BRWG",
              "BGWR",
              "RGBW",
              "RBGW",
              "GRBW",
              "GBRW",
              "BRGW",
              "BGRW"
            ]
          },
          {
            "type": "StringAnyOfByFuncConstrained",
            "name": "ifspeed",
            "default": "KHZ800",
            "allowedValues": [
              "KHZ800",
              "KHZ400"
            ]
          },
          {
            "type": "UInt",
            "name": "brightness",
            "default": 127,
            "minValue": 1,
            "maxValue": 127
          },
          {
            "type": "UInt",
            "name": "mode",
            "default": 0,
            "minValue": 0,
            "maxValue": 80
          },
          {
            "type": "UInt",
            "name": "fxspeed",
            "default": 3000,
            "minValue": 0,
            "maxValue": 65535
          }
        ]
      },
      "CONTAINER": {
        "type": "object",
        "name": "DeviceContainer",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "ArrayOfRegistryItems",
            "name": "items",
            "required": true,
            "regPath": "ROOT"
          }
        ]
      },
      "THINGSPEAK": {
        "type": "object",
        "name": "ThingSpeak",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "_byref_",
            "name": "refreshtime"
          },
          {
            "type": "UInt",
            "name": "firstUpdateAfterSeconds",
            "default": 0,
            "minValue": 0,
            "maxValue": 0
          },
          {
            "type": "String",
            "name": "serverOverride",
            "default": ""
          },
          {
            "type": "StringSizeConstrained",
            "name": "key",
            "required": true,
            "default": "0123456789ABCDEF",
            "minLength": 16,
            "maxLength": 16
          },
          {
            "type": "Object",
            "name": "items",
            "required": true,
            "subtype": {
              "type": "object",
              "name": "ThingSpeakField",
              "unknownPolicy": "Error",
              "emptyPolicy": "Error",
              "fields": [
                {
                  "type": "StringUID_Path",
                  "name": "1"
                },
                {
                  "type": "StringUID_Path",
                  "name": "2"
                },
                {
                  "type": "StringUID_Path",
                  "name": "3"
                },
                {
                  "type": "StringUID_Path",
                  "name": "4"
                },
                {
                  "type": "StringUID_Path",
                  "name": "5"
                },
                {
                  "type": "StringUID_Path",
                  "name": "6"
                },
                {
                  "type": "StringUID_Path",
                  "name": "7"
                },
                {
                  "type": "StringUID_Path",
                  "name": "8"
                }
              ]
            }
          }
        ]
      },
      "HOMEASSISTANT": {
        "type": "object",
        "name": "HomeAssistant",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "modes": [
          {
            "name": "global group mode",
            "conjunctions": [
              {
                "name": "group",
                "required": true
              },
              {
                "name": "items",
                "required": true
              },
              {
                "name": "groups"
              }
            ]
          },
          {
            "name": "individual groups mode",
            "conjunctions": [
              {
                "name": "group"
              },
              {
                "name": "items"
              },
              {
                "name": "groups",
                "required": true
              }
            ]
          }
        ],
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "String",
            "name": "host",
            "required": true,
            "default": ""
          },
          {
            "type": "UInt",
            "name": "port",
            "required": true,
            "default": 1883,
            "minValue": 1,
            "maxValue": 65535
          },
          {
            "type": "AllOfFieldsGroup",
            "name": "credentials",
            "fields": [
              {
                "type": "String",
                "name": "user",
                "default": ""
              },
              {
                "type": "String",
                "name": "pass",
                "default": ""
              }
            ]
          },
          {
            "type": "Object",
            "name": "group",
            "subtype": {
              "type": "object",
              "name": "GlobalGroup",
              "unknownPolicy": "Warn",
              "emptyPolicy": "Warn",
              "fields": [
                {
                  "type": "String",
                  "name": "uid",
                  "required": true,
                  "default": ""
                },
                {
                  "type": "String",
                  "name": "name",
                  "required": true,
                  "default": ""
                }
              ]
            }
          },
          {
            "type": "ArrayOfRegistryItems",
            "name": "items",
            "regPath": "ROOT.HOMEASSISTANT"
          },
          {
            "type": "ArrayOfObjects",
            "name": "groups",
            "subtype": {
              "type": "object",
              "name": "IndividualGroup",
              "unknownPolicy": "Warn",
              "emptyPolicy": "Warn",
              "fields": [
                {
                  "type": "String",
                  "name": "uid",
                  "required": true,
                  "default": ""
                },
                {
                  "type": "String",
                  "name": "name",
                  "required": true,
                  "default": ""
                },
                {
                  "type": "ArrayOfRegistryItems",
                  "name": "items",
                  "regPath": "ROOT.HOMEASSISTANT"
                }
              ]
            }
          }
        ]
      },
      "ACTUATOR": {
        "type": "object",
        "name": "Actuator",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "modes": [
          {
            "name": "h-bridge ab",
            "conjunctions": [
              {
                "name": "hbridgeModeAB",
                "required": true
              },
              {
                "name": "hbridgeModeOC"
              },
              {
                "name": "dir_enableMode"
              },
              {
                "name": "pinBreak"
              }
            ]
          },
          {
            "name": "h-bridge open close",
            "conjunctions": [
              {
                "name": "hbridgeModeAB"
              },
              {
                "name": "hbridgeModeOC",
                "required": true
              },
              {
                "name": "dir_enableMode"
              },
              {
                "name": "pinBreak"
              }
            ]
          },
          {
            "name": "dir/enable",
            "conjunctions": [
              {
                "name": "hbridgeModeAB"
              },
              {
                "name": "hbridgeModeOC"
              },
              {
                "name": "dir_enableMode",
                "required": true
              },
              {
                "name": "pinBreak"
              }
            ]
          },
          {
            "name": "dir/enable/break",
            "conjunctions": [
              {
                "name": "hbridgeModeAB"
              },
              {
                "name": "hbridgeModeOC"
              },
              {
                "name": "dir_enableMode",
                "required": true
              },
              {
                "name": "pinBreak",
                "required": true
              }
            ]
          }
        ],
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "AllOfFieldsGroup",
            "name": "hbridgeModeAB",
            "fields": [
              {
                "type": "HardwarePin",
                "name": "pinA",
                "mode": "OUT"
              },
              {
                "type": "HardwarePin",
                "name": "pinB",
                "mode": "OUT"
              }
            ]
          },
          {
            "type": "AllOfFieldsGroup",
            "name": "hbridgeModeOC",
            "fields": [
              {
                "type": "HardwarePin",
                "name": "pinOpen",
                "mode": "OUT"
              },
              {
                "type": "HardwarePin",
                "name": "pinClose",
                "mode": "OUT"
              }
            ]
          },
          {
            "type": "AllOfFieldsGroup",
            "name": "dir_enableMode",
            "fields": [
              {
                "type": "HardwarePin",
                "name": "pinDir",
                "mode": "OUT"
              },
              {
                "type": "HardwarePin",
                "name": "pinEnable",
                "mode": "OUT"
              }
            ]
          },
          {
            "type": "HardwarePin",
            "name": "pinBreak",
            "mode": "OUT"
          },
          {
            "type": "Object",
            "name": "MinEndStop",
            "subtype": {
              "type": "object",
              "name": "InputPinScheme",
              "unknownPolicy": "Warn",
              "emptyPolicy": "Warn",
              "fields": [
                {
                  "type": "HardwarePin",
                  "name": "pin",
                  "required": true,
                  "mode": "IN"
                },
                {
                  "type": "Bool",
                  "name": "ActiveHigh",
                  "default": false
                }
              ]
            }
          },
          {
            "type": "Object",
            "name": "MaxEndStop",
            "subtype": {
              "type": "object",
              "name": "InputPinScheme",
              "unknownPolicy": "Warn",
              "emptyPolicy": "Warn",
              "fields": [
                {
                  "type": "HardwarePin",
                  "name": "pin",
                  "required": true,
                  "mode": "IN"
                },
                {
                  "type": "Bool",
                  "name": "ActiveHigh",
                  "default": false
                }
              ]
            }
          },
          {
            "type": "UInt",
            "name": "timeoutMs",
            "default": 10000,
            "minValue": 1,
            "maxValue": 0
          }
        ]
      },
      "RELAY_LATCHING": {
        "type": "object",
        "name": "LatchingRelay",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "modes": [
          {
            "name": "direct a b",
            "conjunctions": [
              {
                "name": "directModeAB",
                "required": true
              },
              {
                "name": "directModeOC"
              },
              {
                "name": "dir_enableMode"
              }
            ]
          },
          {
            "name": "direct set reset",
            "conjunctions": [
              {
                "name": "directModeAB"
              },
              {
                "name": "directModeOC",
                "required": true
              },
              {
                "name": "dir_enableMode"
              }
            ]
          },
          {
            "name": "data/enable",
            "conjunctions": [
              {
                "name": "directModeAB"
              },
              {
                "name": "directModeOC"
              },
              {
                "name": "dir_enableMode",
                "required": true
              }
            ]
          }
        ],
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "AllOfFieldsGroup",
            "name": "directModeAB",
            "fields": [
              {
                "type": "HardwarePin",
                "name": "pinA",
                "mode": "OUT"
              },
              {
                "type": "HardwarePin",
                "name": "pinB",
                "mode": "OUT"
              }
            ]
          },
          {
            "type": "AllOfFieldsGroup",
            "name": "directModeOC",
            "fields": [
              {
                "type": "HardwarePin",
                "name": "pinSet",
                "mode": "OUT"
              },
              {
                "type": "HardwarePin",
                "name": "pinReset",
                "mode": "OUT"
              }
            ]
          },
          {
            "type": "AllOfFieldsGroup",
            "name": "dir_enableMode",
            "fields": [
              {
                "type": "HardwarePin",
                "name": "pinData",
                "mode": "OUT"
              },
              {
                "type": "HardwarePin",
                "name": "pinEnable",
                "mode": "OUT"
              }
            ]
          },
          {
            "type": "Object",
            "name": "ResetState",
            "subtype": {
              "type": "object",
              "name": "InputPinScheme",
              "unknownPolicy": "Warn",
              "emptyPolicy": "Warn",
              "fields": [
                {
                  "type": "HardwarePin",
                  "name": "pin",
                  "required": true,
                  "mode": "IN"
                },
                {
                  "type": "Bool",
                  "name": "ActiveHigh",
                  "default": false
                }
              ]
            }
          },
          {
            "type": "Object",
            "name": "SetState",
            "subtype": {
              "type": "object",
              "name": "InputPinScheme",
              "unknownPolicy": "Warn",
              "emptyPolicy": "Warn",
              "fields": [
                {
                  "type": "HardwarePin",
                  "name": "pin",
                  "required": true,
                  "mode": "IN"
                },
                {
                  "type": "Bool",
                  "name": "ActiveHigh",
                  "default": false
                }
              ]
            }
          },
          {
            "type": "UInt",
            "name": "timeoutMs",
            "default": 500,
            "minValue": 1,
            "maxValue": 0
          }
        ]
      },
      "PWM_SERVO": {
        "type": "object",
        "name": "PWM_Servo",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "modes": [
          {
            "name": "ratio value",
            "conjunctions": [
              {
                "name": "minVal",
                "required": true
              },
              {
                "name": "maxVal",
                "required": true
              }
            ]
          },
          {
            "name": "pulse length",
            "conjunctions": [
              {
                "name": "minVal"
              },
              {
                "name": "maxVal"
              }
            ]
          }
        ],
        "constraints": [
          {
            "fieldA": "minVal",
            "type": "<",
            "fieldB": "maxVal"
          },
          {
            "fieldA": "minPulseLength",
            "type": "<",
            "fieldB": "maxPulseLength"
          },
          {
            "fieldA": "minPulseLength",
            "type": "<=",
            "fieldB": "startPulseLength"
          },
          {
            "fieldA": "startPulseLength",
            "type": "<=",
            "fieldB": "maxPulseLength"
          }
        ],
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "HardwarePin",
            "name": "pin",
            "required": true,
            "mode": "OUT"
          },
          {
            "type": "UInt",
            "name": "ch",
            "required": true,
            "default": 0,
            "minValue": 0,
            "maxValue": 7
          },
          {
            "type": "UInt",
            "name": "minPulseLength",
            "default": 1000,
            "minValue": 100,
            "maxValue": 20000
          },
          {
            "type": "UInt",
            "name": "maxPulseLength",
            "default": 2000,
            "minValue": 100,
            "maxValue": 20000
          },
          {
            "type": "UInt",
            "name": "startPulseLength",
            "default": 1500,
            "minValue": 100,
            "maxValue": 20000
          },
          {
            "type": "UInt",
            "name": "autoOffAfterMs",
            "default": 0,
            "minValue": 0,
            "maxValue": 0
          },
          {
            "type": "UInt",
            "name": "pulseLengthOffset",
            "default": 0,
            "minValue": 0,
            "maxValue": 0
          },
          {
            "type": "Float",
            "name": "minVal",
            "default": null,
            "minValue": null,
            "maxValue": null
          },
          {
            "type": "Float",
            "name": "maxVal",
            "default": null,
            "minValue": null,
            "maxValue": null
          }
        ]
      },
      "BUTTON": {
        "type": "object",
        "name": "ButtonInput",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "HardwarePin",
            "name": "pin",
            "required": true,
            "mode": "IN"
          },
          {
            "type": "UInt",
            "name": "debounceMs",
            "default": 30,
            "minValue": 1,
            "maxValue": 0
          },
          {
            "type": "UInt",
            "name": "activeLevel",
            "default": 0,
            "minValue": 0,
            "maxValue": 1
          },
          {
            "type": "String",
            "name": "on_press",
            "default": ""
          }
        ]
      },
      "TEMPLATE": {
        "type": "object",
        "name": "_Template_",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "HardwarePin",
            "name": "pin",
            "required": true,
            "mode": "OUT|IN"
          }
        ]
      }
    },
    "ROOT.TX433": {
      "lc": {
        "type": "object",
        "name": "TX433_Unit_TypeLC",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "modes": [
          {
            "name": "AlphaNumeric ID mode",
            "conjunctions": [
              {
                "name": "anid",
                "required": true
              },
              {
                "name": "hexid"
              }
            ]
          },
          {
            "name": "hex ID mode",
            "conjunctions": [
              {
                "name": "anid"
              },
              {
                "name": "hexid",
                "required": true
              }
            ]
          }
        ],
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "StringSizeConstrained",
            "name": "anid",
            "default": "Id01",
            "minLength": 4,
            "maxLength": 4
          },
          {
            "type": "StringSizeConstrained",
            "name": "hexid",
            "default": "090A0B",
            "minLength": 6,
            "maxLength": 6
          },
          {
            "type": "UInt",
            "name": "grp_btn",
            "default": 0,
            "minValue": 0,
            "maxValue": 3
          },
          {
            "type": "UInt",
            "name": "btn",
            "default": 0,
            "minValue": 0,
            "maxValue": 15
          },
          {
            "type": "UInt",
            "name": "state",
            "default": 0,
            "minValue": 0,
            "maxValue": 1
          }
        ]
      },
      "sfc": {
        "type": "object",
        "name": "TX433_Unit_TypeSFC",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "UInt",
            "name": "ch",
            "default": 0,
            "minValue": 1,
            "maxValue": 4
          },
          {
            "type": "UInt",
            "name": "btn",
            "default": 0,
            "minValue": 1,
            "maxValue": 4
          },
          {
            "type": "UInt",
            "name": "state",
            "default": 0,
            "minValue": 0,
            "maxValue": 1
          }
        ]
      },
      "afc": {
        "type": "object",
        "name": "TX433_Unit_TypeAFC",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "StringAnyOfByFuncConstrained",
            "name": "ch",
            "default": "0",
            "allowedValues": [
              "0",
              "1",
              "2",
              "3",
              "4",
              "5",
              "6",
              "7",
              "8",
              "9",
              "A",
              "B",
              "C",
              "D",
              "E",
              "F"
            ]
          },
          {
            "type": "StringAnyOfByFuncConstrained",
            "name": "btn",
            "default": "0",
            "allowedValues": [
              "0",
              "1",
              "2",
              "3",
              "4",
              "5",
              "6",
              "7",
              "8",
              "9",
              "A",
              "B",
              "C",
              "D",
              "E",
              "F"
            ]
          },
          {
            "type": "UInt",
            "name": "state",
            "default": 0,
            "minValue": 0,
            "maxValue": 1
          }
        ]
      }
    },
    "ROOT.I2C_Master": {
      "SSD1306": {
        "type": "object",
        "name": "Display_SSD1306",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "UInt",
            "name": "width",
            "required": true,
            "default": 128,
            "minValue": 8,
            "maxValue": 128
          },
          {
            "type": "UInt",
            "name": "height",
            "required": true,
            "default": 64,
            "minValue": 8,
            "maxValue": 64
          },
          {
            "type": "UInt",
            "name": "textsize",
            "default": 1,
            "minValue": 1,
            "maxValue": 64
          },
          {
            "type": "StringHexBytes",
            "name": "addr",
            "required": true,
            "default": "3C",
            "byteCount": 1
          },
          {
            "type": "ArrayOfObjects",
            "name": "items",
            "required": true,
            "subtype": "SSD1306_element"
          }
        ]
      },
      "PCF8574x": {
        "type": "object",
        "name": "PCF8574x",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "StringHexBytes",
            "name": "addr",
            "required": true,
            "default": "38",
            "byteCount": 1
          }
        ]
      }
    },
    "ROOT.HOMEASSISTANT": {
      "sensor": {
        "type": "object",
        "name": "HA_Sensor",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "modes": [
          {
            "name": "timedRefresh",
            "conjunctions": [
              {
                "name": "refreshtime",
                "required": true
              },
              {
                "name": "source",
                "required": true
              },
              {
                "name": "event_source"
              }
            ]
          },
          {
            "name": "event",
            "conjunctions": [
              {
                "name": "refreshtime"
              },
              {
                "name": "source",
                "required": true
              },
              {
                "name": "event_source",
                "required": true
              }
            ]
          },
          {
            "name": "manual",
            "conjunctions": [
              {
                "name": "refreshtime"
              },
              {
                "name": "source"
              },
              {
                "name": "event_source"
              }
            ]
          }
        ],
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "_byref_",
            "name": "hass_common"
          },
          {
            "type": "_byref_",
            "name": "consumer"
          },
          {
            "type": "Object",
            "name": "discovery",
            "subtype": {}
          }
        ]
      },
      "binary_sensor": {
        "type": "object",
        "name": "HA_BinarySensor",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "modes": [
          {
            "name": "timedRefresh",
            "conjunctions": [
              {
                "name": "refreshtime",
                "required": true
              },
              {
                "name": "source",
                "required": true
              },
              {
                "name": "event_source"
              }
            ]
          },
          {
            "name": "event",
            "conjunctions": [
              {
                "name": "refreshtime"
              },
              {
                "name": "source",
                "required": true
              },
              {
                "name": "event_source",
                "required": true
              }
            ]
          },
          {
            "name": "manual",
            "conjunctions": [
              {
                "name": "refreshtime"
              },
              {
                "name": "source"
              },
              {
                "name": "event_source"
              }
            ]
          }
        ],
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "_byref_",
            "name": "hass_common"
          },
          {
            "type": "_byref_",
            "name": "consumer"
          },
          {
            "type": "Object",
            "name": "discovery",
            "subtype": {}
          }
        ]
      },
      "switch": {
        "type": "object",
        "name": "HA_Switch",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "_byref_",
            "name": "hass_common"
          },
          {
            "type": "StringUID_Path",
            "name": "target",
            "required": true
          },
          {
            "type": "Bool",
            "name": "momentary",
            "default": false
          },
          {
            "type": "Object",
            "name": "discovery",
            "subtype": {}
          }
        ]
      },
      "button": {
        "type": "object",
        "name": "HA_Button",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "_byref_",
            "name": "hass_common"
          },
          {
            "type": "StringUID_Path",
            "name": "target",
            "required": true
          },
          {
            "type": "Object",
            "name": "discovery",
            "subtype": {}
          }
        ]
      },
      "number": {
        "type": "object",
        "name": "HA_Number",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "_byref_",
            "name": "hass_common"
          },
          {
            "type": "StringUID_Path",
            "name": "target",
            "required": true
          },
          {
            "type": "Object",
            "name": "discovery",
            "subtype": {}
          }
        ]
      },
      "CONTAINER": {
        "type": "object",
        "name": "HA_DeviceContainer",
        "unknownPolicy": "Warn",
        "emptyPolicy": "Warn",
        "fields": [
          {
            "type": "_byref_",
            "name": "disabled_type_uidreq_note_group_items"
          },
          {
            "type": "ArrayOfRegistryItems",
            "name": "items",
            "required": true,
            "regPath": "ROOT.HOMEASSISTANT"
          }
        ]
      }
    }
  },
  "objects": [
    {
      "type": "object",
      "name": "OneWireTempBus",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Error",
      "fields": [
        {
          "type": "_byref_",
          "name": "disabled_uidreq_note_group_items"
        },
        {
          "type": "HardwarePin",
          "name": "pin",
          "required": true,
          "mode": "OUT|IN"
        },
        {
          "type": "ArrayOfObjects",
          "name": "items",
          "required": true,
          "subtype": "OneWireTempDevice"
        }
      ]
    },
    {
      "type": "object",
      "name": "OneWireTempDevice",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Error",
      "fields": [
        {
          "type": "_byref_",
          "name": "disabled_uidreq_note_group_items"
        },
        {
          "type": "StringHexBytes",
          "name": "romid",
          "required": true,
          "default": "00:00:00:00:00:00:00:00",
          "byteCount": 8
        }
      ]
    },
    {
      "type": "object",
      "name": "REGO600_Register",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "type": "_byref_",
          "name": "disabled_uidreq_note_group_items"
        },
        {
          "type": "StringAnyOfByFuncConstrained",
          "name": "regname",
          "required": true,
          "allowedValues": [
            "GT1",
            "GT2",
            "GT3",
            "GT4",
            "GT5",
            "GT6",
            "GT8",
            "GT9",
            "GT10",
            "GT11",
            "GT3x",
            "P3",
            "COMP",
            "EL3",
            "EL6",
            "P1",
            "P2",
            "VXV",
            "ALARM"
          ]
        }
      ]
    },
    {
      "type": "object",
      "name": "SSD1306_element",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "type": "Bool",
          "name": "disabled",
          "default": false
        },
        {
          "type": "StringUID",
          "name": "uid",
          "required": true
        },
        {
          "type": "UInt",
          "name": "x",
          "required": true,
          "default": 0,
          "minValue": 0,
          "maxValue": 128
        },
        {
          "type": "UInt",
          "name": "y",
          "required": true,
          "default": 0,
          "minValue": 0,
          "maxValue": 64
        },
        {
          "type": "String",
          "name": "label",
          "default": ""
        },
        {
          "type": "StringUID_Path",
          "name": "source"
        }
      ]
    }
  ],
  "ByReference": [
    {
      "type": "FieldsGroup",
      "name": "disabled_type_uidreq_note_group_items",
      "fields": [
        {
          "type": "_byref_",
          "name": "disabled_uidreq_note_group_items"
        },
        {
          "type": "String",
          "name": "type",
          "required": true
        }
      ]
    },
    {
      "type": "OneOfFieldsGroup",
      "name": "refreshtime",
      "required": true,
      "fields": [
        {
          "type": "UInt",
          "name": "refreshtimems",
          "default": 1,
          "minValue": 0,
          "maxValue": 0
        },
        {
          "type": "Float",
          "name": "refreshtimesec",
          "default": null,
          "minValue": null,
          "maxValue": null
        },
        {
          "type": "Float",
          "name": "refreshtimemin",
          "default": null,
          "minValue": null,
          "maxValue": null
        }
      ]
    },
    {
      "type": "FieldsGroup",
      "name": "hass_common",
      "fields": [
        {
          "type": "String",
          "name": "name",
          "required": true,
          "default": ""
        },
        {
          "type": "String",
          "name": "hass_uid",
          "required": true,
          "default": ""
        },
        {
          "type": "String",
          "name": "hass_prev_uid",
          "default": ""
        }
      ]
    },
    {
      "type": "FieldsGroup",
      "name": "consumer",
      "fields": [
        {
          "type": "StringUID_Path",
          "name": "source"
        },
        {
          "type": "StringUID_Path",
          "name": "event_source"
        },
        {
          "type": "_byref_",
          "name": "refreshtime"
        }
      ]
    },
    {
      "type": "FieldsGroup",
      "name": "disabled_uidreq_note_group_items",
      "fields": [
        {
          "type": "Bool",
          "name": "disabled",
          "default": false
        },
        {
          "type": "StringUID",
          "name": "uid",
          "required": true
        },
        {
          "type": "String",
          "name": "note"
        }
      ]
    }
  ]
};