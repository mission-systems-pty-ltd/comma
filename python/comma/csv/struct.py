import numpy as np
from comma.csv.common import *


class struct_error(Exception):
    pass


class struct:
    default_field_name = 'comma_struct_default_field_name_'

    def __init__(self, concise_fields, *concise_types):
        self.concise_types = concise_types
        self.concise_fields = self._fill_blanks(concise_fields)
        self._check_fields_conciseness()
        self.dtype = np.dtype(zip(self.concise_fields, self.concise_types))
        self.fields = self._full_xpath_fields()
        self.types = self._basic_types()
        self.shorthand = self._shorthand()
        self.format = format_from_types(self.types)
        self.flat_dtype = np.dtype(zip(self.fields, self.types))
        unrolled_types = unrolled_types_of_flat_dtype(self.flat_dtype)
        self.unrolled_flat_dtype = structured_dtype(','.join(unrolled_types))
        self.type_of_field = dict(zip(self.fields, self.types))
        leaves = tuple(xpath.split('/')[-1] for xpath in self.fields)
        self.ambiguous_leaves = set(leaf for leaf in leaves if leaves.count(leaf) > 1)
        self.xpath_of_leaf = self._xpath_of_leaf(leaves)

    def __call__(self, size=1):
        return np.empty(size, dtype=self)

    def to_tuple(self, s):
        if s.dtype != self.dtype:
            msg = "expected {}, got {}".format(repr(self.dtype), repr(s.dtype))
            raise struct_error(msg)
        if not (s.shape == (1,) or s.shape == ()):
            msg = "expected a scalar or 1d array with size=1, got {}".format(s.shape)
            raise struct_error(msg)
        return s.view(self.unrolled_flat_dtype).item()

    def expand_shorthand(self, compressed_fields):
        if isinstance(compressed_fields, basestring):
            compressed_fields = compressed_fields.split(',')
        expand = self.shorthand.get
        field_tuples = map(lambda name: expand(name) or (name,), compressed_fields)
        return sum(field_tuples, ())

    def _fill_blanks(self, fields):
        if isinstance(fields, basestring):
            fields = fields.split(',')
        ntypes = len(self.concise_types)
        if len(fields) > ntypes:
            fields_without_type = ','.join(fields.split(',')[ntypes:])
            msg = "missing types for fields '{}'".format(fields_without_type)
            raise struct_error(msg)
        omitted_fields = [''] * (ntypes - len(fields))
        fields_without_blanks = []
        for index, field in enumerate(fields + omitted_fields):
            if field:
                nonblank_field = field
            else:
                nonblank_field = '{}{}'.format(struct.default_field_name, index)
            fields_without_blanks.append(nonblank_field)
        return fields_without_blanks

    def _check_fields_conciseness(self):
        for field in self.concise_fields:
            if '/' in field:
                msg = "expected fields without '/', got '{}'".format(self.concise_fields)
                raise struct_error(msg)

    def _full_xpath_fields(self):
        fields = []
        for name, type in zip(self.concise_fields, self.concise_types):
            if isinstance(type, struct):
                fields_of_type = [name + '/' + field for field in type.fields]
                fields.extend(fields_of_type)
            else:
                fields.append(name)
        return tuple(fields)

    def _basic_types(self):
        types = []
        for type in self.concise_types:
            if isinstance(type, struct):
                types.extend(type.types)
            else:
                types.append(type)
        return tuple(types)

    def _shorthand(self):
        shorthand = {}
        for name, type in zip(self.concise_fields, self.concise_types):
            if not isinstance(type, struct):
                continue
            fields_of_type = [name + '/' + field for field in type.fields]
            shorthand[name] = tuple(fields_of_type)
            for subname, subfields in type.shorthand.iteritems():
                xpath = name + '/' + subname
                shorthand[xpath] = tuple(name + '/' + field for field in subfields)
        return shorthand

    def _xpath_of_leaf(self, leaves):
        xpath_of_leaf = dict(zip(leaves, self.fields))
        for ambiguous_leaf in self.ambiguous_leaves:
            del xpath_of_leaf[ambiguous_leaf]
        return xpath_of_leaf
