// jsValue
addWrapper(function(Module, api) {
    var typedValues = [
        'TypedValueInteger',
        'TypedValueBoolean',
        'TypedValueFloat',
        'TypedValueColor3',
        'TypedValueColor4',
        'TypedValueVector2',
        'TypedValueVector3',
        'TypedValueVector4',
        'TypedValueMatrix33',
        'TypedValueMatrix44',
        'TypedValueString',
        'TypedValueIntegerArray',
        'TypedValueBooleanArray',
        'TypedValueFloatArray',
        'TypedValueStringArray'
    ];

    function iterateTypedValues(cb) {
        for (var i = 0; i < typedValues.length; i++) {
            var typedValue = typedValues[parseInt(i, 10)];
            cb && cb(typedValue);
        }
    }

    /** Setup the Value class */
    api.Value = wrapperFactory(Module.Value);

    /** Setup the typedValue classes */
    iterateTypedValues(function(typedValue) {
        api[String(typedValue)] = wrapperFactory(Module[typedValue]);
    });
});
