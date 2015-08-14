_empty_svg = '<?xml version="1.0" encoding="UTF-8" standalone="no"?><svg xmlns="http://www.w3.org/2000/svg" version="1.0" width="32" height="32" viewBox="0 0 32 32"></svg>'

import json
import os
import warnings

class StrictDict(dict):
    """General class for model data (parameters and variables).

       Class is inherited from builtins dict and implement data type checking.
    """

    def __init__(self, *args, **kwargs):
        self.update(*args, **kwargs)
        self._data_types = dict()

    def data_types(self):
        """Return dictionary with defined data types."""
        return self._data_types

    def declare(self, name, data_type):
        """Declare new parameters which will be stored in dictionary.

        Args:
            name: Parameter name.
            data_type: Data type of stred value.
        """

        self._data_types[name] = data_type

    def fulfilled(self):
        """Check that all items are set.

        Returns:
            Return True if all defined items in _data_types are set,
            otherwise return False.
        """

        for name in self._data_types.keys():
            if not name in self.keys(): return False
        return True

    def __setitem__(self, name, value):
        if not name in self._data_types:
            raise KeyError('Value with name "{0}" is not declared!'.format(name))

        data_type = self._data_types[name]
        if (type(value) != data_type and
            not (data_type == float and type(value) == int)):
            raise TypeError('Value data type do not correspond with defined data type!')

        self.update({name : value})

class Parameters(StrictDict):
    """General class for model parameters.

       Class is inherited from StrictDict and implement default values.
    """

    def declare(self, name, data_type, default=None):
        """Declare new parameters which will be stored in dictionary.

        Args:
            name: Parameter name.
            data_type: Data type of stred value.
            default: Optional argument declared default value of parameter.
        """

        if default:
            if (type(default) != data_type and
                not (data_type == float and type(default) == int)):
                raise TypeError('Value data type do not correspond with defined data type!')

            self.update({name : default})
        self._data_types[name] = data_type

class ModelData:
    """General class collect all model data.

        Attributes:
            parameters: Instance of variant Parameters class stored input values for model.
            variables: Instance of variant StrictDict class stored  output values (results) of model.
            info: Dictionary stored additional information described model.
    """

    def __init__(self):
        self.parameters = Parameters()
        self.variables = StrictDict()
        self.info = {'_geometry' : _empty_svg}

class ModelBase:
    """General class described model."""

    def __init__(self):
        self._data = ModelData()
        self.declare()

    @property
    def parameters(self):
        """Model input parameters."""
        return self._data.parameters

    @parameters.setter
    def parameters(self, value):
        raise RuntimeError('Object cannot be overwrite!')

    @property
    def variables(self):
        """Model output variables."""
        return self._data.variables

    @variables.setter
    def variables(self, value):
        raise RuntimeError('Object cannot be overwrite!')

    @property
    def info(self):
        """Additional information described model."""
        return self._data.info

    @info.setter
    def info(self, value):
        self._data.info = value

    def data(self):
        """Return model data."""
        return self._data

    def solved(self):
        """Solution state of the model.

        Returns:
            Return True if declared variables are set, otherwise return False.
        """
        return self._data.variables.fulfilled()

    def clear(self):
        """Clear model output variables."""
        self._data.variables.clear()

    def declare(self):
        """Method for declaration of model parameters and variables.

        class Model(ModelBase):
            def declare(self):
                self.parameters.declare('p', float, default=0.0)
                self.variables.declare('v', float)
            ...
        """
        warnings.warn("Method declare() should be overrided!", RuntimeWarning)

    def create(self):
        """Method creates model from input parameters."""
        pass

    def solve(self):
        """Method for solution of model and variables calculation."""
        warnings.warn("Method solve() should be overrided!", RuntimeWarning)

    def load(self, file_name):
        """Load model data file.

        Args:
            file_name: Name of data file with model data.
        """

        with open(file_name, 'r') as infile:
            data = json.load(infile)
            self._data.parameters = StrictDict(data['parameters'])
            self._data.variables = StrictDict(data['variables'])
            self._data.info = data['info']

    def save(self, file_name):
        """Save model data to the file.

        Args:
            file_name: Name of new model data file.
        """

        directory = os.path.dirname(file_name)
        if (directory and not os.path.isdir(directory)):
            os.makedirs(directory)

        with open(file_name, 'w') as outfile:
            data = {'parameters' : self._data.parameters,
                    'variables' : self._data.variables,
                    'info' : self._data.info}

            json.dump(data, outfile)

if __name__ == '__main__':
    import pythonlab

    class Model(ModelBase):
        def declare(self):
            self.parameters.declare('p', float, default = 1.0)
            self.variables.declare('v', float)

    model = Model()
    model.parameters['p'] = 2.0
    model.variables['v'] = 2.0

    file_name = '{0}/model.pickle'.format(pythonlab.tempname())
    model.save(file_name)
    model.load(file_name)
    print(model.variables)