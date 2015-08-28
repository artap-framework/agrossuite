from variant.model import ModelBase
from variant.criterion import Criterions
from variant.htcondor import Solver, Process, Job, Connection

from collections import OrderedDict
from tempfile import TemporaryDirectory
from subprocess import Popen, PIPE
from shutil import make_archive, move, copy2
from time import sleep
from importlib import machinery

import agros2d
import os
import zipfile
import json
import time

class ModelDictionary:
    """General class for model collection.

        Attributes:
            dictionary: Dictionary for models (pairs name : model).
            description: Dictionary for information about models.
            criterions: Criterions alows use filter for models.
    """

    def __init__(self, models=None):
        """Initialization of dictionary.

        Args:
            models: List of models or dictionary of pairs name : model (default is None).
        """

        self._dictionary = OrderedDict()
        self._description = dict()
        self._model_class = ModelBase
        self._data_file = None
        self._problem_file = None

        self.criterions = Criterions()

        if (models and isinstance(models, dict)):
            for name, model in models.items():
                self.add_model(model, name)

        if (models and isinstance(models, list)):
            for model in models:
                self.add_model(model)

    @property
    def dictionary(self):
        """Return dictionary with pairs name : model."""
        return self._dictionary

    @dictionary.setter
    def dictionary(self, value):
        raise RuntimeError('Object cannot be overwrite!')

    @property
    def description(self):
        """Return description for stored models."""
        return self._description

    @description.setter
    def description(self, value):
        self._description = value

    def models(self, filter=False, only_solved=False):
        """Return list of stored models.

        Args:
            filter: If True, return models complying with criterions collected in self.criterions.
            only_solved: If True, return only solved models (default is False).
        """

        if (not only_solved and not filter):
            return list(self._dictionary.values())

        models = []
        if filter:
            for name, model in self._dictionary.items():
                if self.criterions.validate(model):
                    models.append(model)

        if only_solved:
            for model in list(self._dictionary.values()):
                if model.solved(): models.append(model)

        return models

    def parameters(self):
        """Return dictionary of parameters and its values."""

        names = self._model_class().parameters.data_types().keys()
        parameters = {}
        for name in names:
            parameters[name] = self.parameter(name)
        return parameters

    def parameter(self, parameter):
        """Return list of values for specific model parameter.

        Args:
            parameter: Parameter name.
        """

        values = []
        for model in self._dictionary.values():
            values.append(model.parameters[parameter])
        return values

    def variables(self):
        """Return dictionary of variables and its values."""

        names = self._model_class().variables.data_types().keys()
        variables = {}
        for name in names:
            variables[name] = self.variable(name)
        return variables

    def variable(self, variable):
        """Return list of values for specific model variable.

        Args:
            variable: Variable name.
        """

        values = []
        for model in self._dictionary.values():
            if model.solved():
                values.append(model.variables[variable])
            else:
                values.append(None)
        return values

    def info(self, item):
        """Return list of values for specific info item.

        Args:
            item: Info item name.
        """

        values = []
        for model in self._dictionary.values():
            values.append(model.info[item])
        return values

    def add_model(self, model, name=None):
        """Add new model to dictionary.

        Args:
            model: Model object inherited from ModelBase class.
            name: Model name used as dictionary key and model data file name (default is None)
        """

        if model.__class__.__base__ != ModelBase:
            raise TypeError('Model class have to be inherited from ModelBase!')

        if not self._dictionary:
            self._model_class = model.__class__

        if (self._dictionary and (self._model_class.__name__ != model.__class__.__name__)):
            raise TypeError('Model class must be {0}.'.format(self._model_class))

        if name in self._dictionary.keys():
            raise KeyError('Model with key "{0}" already exist.'.format(name))

        if not name:
            if hasattr(self, '_model_index'): self._model_index += 1
            else: self._model_index = 0

            name = 'model_{0:0{1}d}'.format(self._model_index, 9)

        self._dictionary[name] = model

    def remove_model(self, name):
        """Remove existing model from dictionary.

        Args:
            name: Model name.
        """
        del self._dictionary[name]

    def load_from_file(self, data_file):
        """Clear dictionary and load models from dictionary data file.

        Args:
            data_file: Name of dictionary data file.
        """

        with zipfile.ZipFile(data_file, 'r') as zip_file:
            self.clear()

            self._description = json.loads(zip_file.read('problem.desc').decode())
            with TemporaryDirectory() as temp:
                zip_file.extract('problem.py', temp)
                loader = machinery.SourceFileLoader('problem', '{0}/problem.py'.format(temp))
                self._model_class = eval('loader.load_module().{0}'.format(self.description['model_class']))
                del self.description['model_class']

            for file in sorted(zip_file.namelist()):
                if not file.endswith('data'): continue

                model = self._model_class()
                print(zip_file.read(file))
                model.deserialize(str(zip_file.read(file)))
                self._dictionary[os.path.splitext(file)[0]] = model

            zip_file.close()

        self._model_index = len(self._dictionary)-1

    def load(self):
        with open('{0}/problem.desc'.format(self._data_file), 'r') as infile:
            self._description = json.load(infile)

        loader = machinery.SourceFileLoader('problem', '{0}/problem.py'.format(self._data_file))
        self._model_class = eval('loader.load_module().{0}'.format(self.description['model_class']))
        
        for file_name in sorted(os.listdir(self._data_file)):
            if not file_name.endswith('data'): continue

            model = self._model_class()
            model.load('{0}/{1}'.format(self._data_file, file_name))
            self._dictionary[os.path.splitext(file_name)[0]] = model

    def save_to_file(self, data_file, problem_file):
        """Save all models to dictionary data file (standard extension is .opt).

        Args:
            data_file: Name of dictionary data file (standard extension is .opt).
            problem_file: Name of Python script with model class definition.
        """
        # TODO: Save data file without model class script.

        with zipfile.ZipFile(data_file, 'w', zipfile.ZIP_DEFLATED) as zip_file:
            for name, model in self._dictionary.items():
                zip_file.writestr('{0}.data'.format(name), model.serialize())

            description = dict(list(self.description.items()) + [('model_class', self._model_class.__name__)])
            zip_file.writestr('problem.desc', json.dumps(description))
            zip_file.write(problem_file, arcname='problem.py')
            zip_file.close()

        self._data_file = data_file
        self._problem_file = problem_file

    def save(self, problem_file):
        path = agros2d.cachedir('analyses/{0}'.format(int(time.time())))
        for name, model in self._dictionary.items():
            model.save('{0}/{1}.data'.format(path, name))

        description = dict(list(self.description.items()) + [('model_class', self._model_class.__name__)])
        model = self._model_class()
        for name, type in model.parameters.data_types().items():
            description[name] = str(type)

        with open('{0}/problem.desc'.format(path), 'w') as outfile:
            json.dump(description, outfile)

        copy2(problem_file, '{0}/problem.py'.format(path))

        self._data_file = path
        self._problem_file = problem_file

    def solve(self, recalculate=False, save=False):
        """Solve models stored in directory.

        Args:
            recalculate: Recalculate already solved models (default is False).
            save: Save models data files after model solution (default is False).
        """

        if (save and not self._data_file or
            save and not self._problem_file):
            raise RuntimeError('Dictionary must be saved before solution with continuous model saving!')

        if not save:
            for name, model in self._dictionary.items():
                solve_model = recalculate or not model.solved()
                if not solve_model: continue

                model.create(); model.solve()
        else:
            with TemporaryDirectory() as temp:
                with zipfile.ZipFile(self._data_file, 'r') as zip_file:
                    zip_file.extractall(temp)
                    zip_file.close()

                for name, model in self._dictionary.items():
                    solve_model = recalculate or not model.solved()
                    if not solve_model: continue

                    model.create(); model.solve()
                    model.save('{0}/{1}.pickle'.format(temp, name))

                    with TemporaryDirectory() as archive_temp:
                        temp_file = '{0}/{1}'.format(archive_temp, os.path.basename(self._data_file).split('.')[0])
                        make_archive(base_name=temp_file, format='zip', root_dir=temp)
                        move('{0}.zip'.format(temp_file), self._data_file)

    def clear(self):
        """Clear models dictionary."""
        self._dictionary.clear()
        self._description.clear()
        self._model_class = ModelBase

class ModelDictionaryExternal(ModelDictionary):
    """ModelDictionary class allows use external solver/interpreter.

        Class is inherited from ModelDictionary.

        Attributes:
            solver: Path to external solver (default solver is None).
            solver_parameters: List of solver parameters.
            solver_inputs: List of solver input files or code (default is None).
            wait_for_solution: If False, script will not wait for solver termination (default is True).
    """

    def __init__(self, models=None):
        """Initialization of model dictionary."""

        ModelDictionary.__init__(self, models)

        self.solver = None
        self.solver_parameters = list()
        self.solver_inputs = None
        self.wait_for_solution = True

        self._output = []
        self._errors = []

    def output(self):
        """Return output from solver."""
        return self._output

    def errors(self):
        """Return errors from solver."""
        return self._errors

    def solve(self, recalculate=False, save=True):
        """Solve models stored in directory.

        Args:
            recalculate: Recalculate already solved models (default is False).
            save: Save models data files after model solution (default is True).
        """

        if not self._data_file:
            raise RuntimeError('Dictionary must be saved before solution!')

        if not self.solver_inputs:
            code = 'from variant import ModelDictionary;'
            code += 'md = ModelDictionary();md.load(\"{0}\");'.format(self._data_file)
            code += 'md.solve({0},{1});'.format(recalculate, save)
            if not save:
                code += 'md.save(\"{0}\", \"{1}\");'.format(self._data_file, self._problem_file)

            command = [self.solver] + self.solver_parameters + [code]
        else:
            command = [self.solver] + self.solver_parameters + self.solver_inputs

        self.description['command'] = ' '.join(command)
        if self.wait_for_solution:
            process = Popen(command, stdout=PIPE,
                                     stderr=PIPE)

            stdout, stderr = process.communicate()
            self._output.append(stdout)
            self._errors.append(stderr)

            self.load(self._data_file)
        else:
            process = Popen(command, close_fds=True)

class ModelDictionaryCondor(ModelDictionaryExternal):
    """ModelDictionary class allows use external solver/interpreter on HTCondor cluster.

        Class is inherited from ModelDictionaryExternal.
    """

    def __init__(self, models=None):
        """Initialization of model dictionary.    """

        ModelDictionaryExternal.__init__(self, models)

        self._cluster = None
        self._processes = []

    def connect(self, hostname, username, key_filename):
        """Create connection with cluster.

        Args:
            hostname: Address to cluster submiter.
            username: User name for connection.
            key_filename: Path to SSH public key.
        """

        self.condor = Connection(hostname=hostname, username=username,
                                 key_filename=key_filename)
        self.condor.connect()

    def _create_process(self, name, model, job_directory):
        code = 'from problem import {0};model={0}();'.format(self._model_class.__name__)
        code += 'model.load(\'{0}.pickle\');model.create();'.format(name)
        code += ' model.solve();model.save(\'{0}.pickle\')'.format(name)

        file_name = '{0}/{1}.py'.format(job_directory, name)
        with open(file_name, 'w') as script:
            script.write(code)

        return Process('{0} {1}.py'.format(' '.join(self.solver_parameters), name))

    def solve(self, recalculate=False, save=True):
        """Solve models stored in directory.

        Args:
            recalculate: Recalculate already solved models (default is False).
            save: Save models data files after model solution (default is True).
        """

        if not self._data_file:
            raise RuntimeError('Dictionary must be saved before solution!')

        with TemporaryDirectory() as temp:
            # extract dictionary
            with zipfile.ZipFile(self._data_file, "r") as zip_file:
                zip_file.extractall(temp)

            # crete and submit job
            job = Job(name=self._data_file)
            if self.solver.__class__ == Solver:
                job.solver = self.solver
            else:
                job.executable = self.solver

            for name, model in self._dictionary.items():
                solve_model = recalculate or not model.solved()
                if not solve_model: continue

                process = self._create_process(name, model, temp)
                process.input_files = ['{0}/problem.py'.format(temp),
                                       '{0}/{1}.pickle'.format(temp, name),
                                       '{0}/{1}.py'.format(temp, name)]
                job.add_process(process)

            job_directory = job.create(temp)
            self._cluster, self._processes = self.condor.submit_job(job_directory)

            # withdraw job and update dictionary
            if self.wait_for_solution:
                while not self.condor.job_completed(self._cluster):
                    sleep(1)

                self.condor.withdraw_job(self._cluster)
                for name, model in self._dictionary.items():
                    model.load('{0}/{1}.pickle'.format(job_directory, name))

                self.save(self._data_file, self._problem_file)

if __name__ == '__main__':
    from variant.test_functions import QuadraticFunction
    from variant.criterion import RangeCriterion
    from variant.functional import Functional
    import pythonlab

    """
    md = ModelDictionaryCondor(QuadraticFunction)
    md.solver = Solver('agros2d') #architectures=['X86_64', 'INTEL']
    md.solver.add_solver_path('/usr/bin/xvfb-run -a /home/fmach/agros2d/agros2d_solver', 'LINUX')
    md.solver.add_solver_path('d:\Seafile\SW_EU507\Agros2D\Solver.exe', 'WINDOWS')
    md.solver_parameters = ['-l', '-s']

    #md.solver = '/usr/bin/xvfb-run'
    #md.solver_parameters = ['-a', '/home/fmach/agros2d/agros2d_solver', '-l', '-s']
    for x in range(10):
        model = QuadraticFunction()
        model.parameters['x'] = x
        md.add_model(model)

    data_file = pythonlab.tempname('opt')
    md.save(data_file, 'test_functions/quadratic_function.py')
    md.connect(hostname='tesla.fel.zcu.cz', username='fmach', key_filename='/home/fmach/.ssh/id_rsa')
    #md.condor.use_tape_archiver = False
    #md.wait_for_solution = False
    #md.condor.clean_remote_directory = False
    md.solve()

    print(md.variables())
    """

    md = ModelDictionary() #ModelDictionaryExternal()
    md.solver = '{0}/agros2d_solver'.format(pythonlab.datadir())
    for x in range(20):
        model = QuadraticFunction()
        model.parameters['x'] = x
        md.add_model(model)

    data_file = pythonlab.tempname('opt')
    md.solve(save=False)
    md.save('test_functions/quadratic_function.py')
    print('{0}/{1}'.format(len(md.models()), len(md.models(only_solved=True))))
    md.load()
    print('{0}/{1}'.format(len(md.models()), len(md.models(only_solved=True))))

    md.criterions.add_criterion(RangeCriterion('75<=F=<80'), [Functional('F'), 50, 80])
    for model in md.models(filter=True):
        print('F={0}'.format(model.variables['F']))

    pythonlab.chart(md.parameter('x'),
                    md.variable('F'), 'x', 'F')

    print(md.parameters())
    print(md.variables())
    print(md.description)