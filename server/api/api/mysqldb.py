import json
import time
import mysql.connector


class MySqlDB():
    """
        This clase helps with MySQL database connection and data fetching
    """

    db = None
    cur = None
    db_mappings = None

    def __init__(
        self,
        conf_file_path=None,
        host=None,
        port=None,
        user=None,
        passwd=None,
        database=None
    ):
        if conf_file_path:
            __conf = self.__load_json_conf_file(conf_file_path)
            __db_conf = __conf['configurations']
            self.db_mappings = __conf['mappings']
        else:
            __db_conf = {
                "host": host,
                "port": port,
                "user": user,
                "passwd": passwd,
                "database": database
            }

        if all(__db_conf.values()):
            self.__set_connector(__db_conf)
            self.__set_cursor()
        else:
            raise Exception('Provide the configuration info')

    def __load_json_conf_file(self, file_name):
        """
            Loads database configurations from json file
        """

        with open(file_name, 'r') as conf_file:
            return json.load(conf_file)

    def __set_connector(self, db_conf, tries=20, delay=5):
        """
            Attempts to set the database connector
        """

        max_tries = tries

        while tries:
            try:
                self.db = mysql.connector.connect(**db_conf)

                print('Connection to {} successful!'.format(db_conf['host']))

                break
            except:
                print(
                    '[{}/{}] Error: cannot connect to {}, retrying in {}s...'.format(
                        tries,
                        max_tries,
                        db_conf['host'],
                        delay
                    )
                )

                tries -= 1
                time.sleep(delay)
        else:
            raise Exception('Error: cannot connect to the database')

    def __set_cursor(self):
        """
            Sets the cursor to a databse resource
        """

        if self.db:
            self.cur = self.db.cursor(dictionary=True)

    def execute(self, query):
        """
            Executes a single query
        """

        try:
            self.cur.execute(query)
        except Exception as e:
            print('Error: {}'.format(e))

    def get_all(self, _id):
        """
            Gets all information about the product of the provided id and
            maps the column names according to the configuration file specification.
        """

        # columns from the database's table
        table_columns = self.db_mappings['columns'].values()

        query = "SELECT {} FROM {} WHERE {} = {}".format(
            ", ".join("`{0}`".format(column) for column in table_columns),
            self.__add__grave_accents(self.db_mappings['table']),
            self.__add__grave_accents(self.db_mappings['columns']['id']),
            _id
        )

        print("Executing query: {}".format(query))

        self.execute(query)

        result = self.cur.fetchone()

        # re-map back the the API abstracted names
        swap_mapping_columns = {
            value: key for key,
            value in self.db_mappings['columns'].items()
        }

        try:
            for key in result:
                if key in swap_mapping_columns:
                    result[swap_mapping_columns[key]] = result.pop(key)
        except:
            return {}

        return result

    def __add__grave_accents(self, string):
        """
            Adds `s to the both sides of the input string (beggining and end)
        """

        return "".join("`{0}`".format(string))
