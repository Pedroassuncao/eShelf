"""
    This script sets up an API which retrives information
    from a MySQL database.
    The retrived information corresponds to the product
    we wanna request by indicating its id.
    The API sends back the product information in json format.
"""
import json
from mysqldb import MySqlDB
from flask import Flask
from flask_restful import Resource, Api

eshelf_db = MySqlDB('./conf/db_conf.json')
# eshelf_db = MySqlDB('../../conf/db_conf.json')

app = Flask(__name__)
api = Api(app)


class GetProductInfo(Resource):
    """
        Gets information of a product from the database and returns it as json.
    """

    def get(self, product_id):
        """
            Returns, in json, all the information about
            the product specified by the product_id.
            If the product does not exist in the database
            the response will be an empty json.
        """

        # TODO properly sanitize product_id
        # so we don't have edgy code injections

        return eshelf_db.get_all(product_id)


api.add_resource(GetProductInfo, '/products/<string:product_id>')

with open('./conf/api_conf.json', 'r') as conf_file:
    conf = json.load(conf_file)
    app.run(host=conf['host'], port=conf['port'], debug=False)
