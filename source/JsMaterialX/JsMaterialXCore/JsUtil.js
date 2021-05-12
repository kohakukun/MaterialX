// jsUtil
addWrapper(function(Module, api) {
    // TODO: Wrap functions and provide default args
    api.getVersionString = Module.getVersionString;
    api.createValidName = function(str, char) {
        return Module.createValidName(str, char.charCodeAt(0));
    };

    api.isValidName = Module.isValidName;
    api.incrementName = Module.incrementName;

    api.getVersionIntegers = function() {
        return Module.getVersionIntegers();
    };
});
