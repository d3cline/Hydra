from rest_framework import parsers, renderers
from rest_framework import viewsets
from rest_framework.authtoken.models import Token
from rest_framework.authtoken.serializers import AuthTokenSerializer
from rest_framework.throttling import AnonRateThrottle
from rest_framework.views import APIView

from cropwatch.apps.metrics.models import *

from django.core.exceptions import ObjectDoesNotExist
from django.utils.timezone import activate
from rest_framework import serializers
from rest_framework import status
from rest_framework.decorators import api_view, permission_classes
from rest_framework.response import Response

from cropwatch.apps.metrics.permissions import IsBot
from cropwatch.apps.metrics.tasks import *



# Serializers define the API representation.
class UserSerializer(serializers.HyperlinkedModelSerializer):
    class Meta:
        model = User
        fields = ('url', 'username', 'email', 'is_staff')


# ViewSets define the view behavior.
class UserViewSet(viewsets.ModelViewSet):
    queryset = User.objects.all()
    serializer_class = UserSerializer


# we override the default auth because there is no throttle. 
# using anon throttle
class ObtainAuthToken(APIView):
    throttle_classes = (AnonRateThrottle,)
    permission_classes = ()
    parser_classes = (parsers.FormParser, parsers.MultiPartParser, parsers.JSONParser,)
    renderer_classes = (renderers.JSONRenderer,)
    serializer_class = AuthTokenSerializer

    def post(self, request, *args, **kwargs):
        serializer = self.serializer_class(data=request.data,
                                           context={'request': request})
        serializer.is_valid(raise_exception=True)
        user = serializer.validated_data['user']
        token, created = Token.objects.get_or_create(user=user)
        return Response({'token': token.key})

obtain_auth_token = ObtainAuthToken.as_view()


def is_night(now, start, end):
    if start <= end:
        return start <= now < end
    else:  # over midnight e.g., 23:30-04:15
        return start <= now or now < end

class iotv1Serializer(serializers.ModelSerializer):
    ip = serializers.CharField(max_length=45)  # no corresponding model property.
    rssi = serializers.CharField(max_length=4)
    class Meta:
        model = SensorReading
        fields = ['rtd', 'ec', 'do', 'ph', 'orp', 'ip', 'rssi']


class ioTankAddSerializer(serializers.Serializer):
    add = serializers.CharField(max_length=5)
    def validate_add(self, value):
        if 'true' not in value.lower():
            raise serializers.ValidationError("add not true")
        return value


class ioTankDeleteSerializer(serializers.Serializer):
    delete = serializers.CharField(max_length=5)
    uuid = serializers.UUIDField()
    def validate_delete(self, value):
        if 'true' not in value.lower():
            raise serializers.ValidationError("delete not true")
        return value


class ioTankListSerializer(serializers.Serializer):
    list_tokens = serializers.CharField(max_length=5)
    def validate_list_tokens(self, value):
        if 'true' not in value.lower():
            raise serializers.ValidationError("list_tokens not true")
        return value


@api_view(['POST'])
@permission_classes((IsBot,))
def v1_ioTank_add(request):
    if request.method == 'POST':
        serializer = ioTankAddSerializer(data=request.data)
        if serializer.is_valid():
            try:
                newio = ioTank(owner=request.user)
                user = User.objects.create_user(str(newio)[:30])
                newio.bot_user_id = user.id
                newio.save()
                Token.objects.get_or_create(user=user)
                return Response(timezone.now(), status=status.HTTP_201_CREATED)
            except:
                return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)
        else:
            return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)


@api_view(['POST'])
@permission_classes((IsBot,))
def v1_ioTank_delete(request):
    if request.method == 'POST':
        serializer = ioTankDeleteSerializer(data=request.data)
        if serializer.is_valid():
            uuid = serializer.validated_data.get('uuid')

            return Response(timezone.now(), status=status.HTTP_200_OK)
        else:
            return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)
    else:
        return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)


@api_view(['POST'])
@permission_classes((IsBot,))
def v1_ioTank_list(request):
    if request.method == 'POST':
        serializer = ioTankListSerializer(data=request.data)
        if serializer.is_valid():
            bots = ioTank.objects.filter(owner=request.user)
            out = {}

            for b in bots:
                token = Token.objects.get(user=b.bot_user)
                out[str(b.id)] = str(token)

            return Response(out, status=status.HTTP_200_OK)
        else:
            return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)
    else:
        return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)


@api_view(['POST'])
@permission_classes((IsBot,))
def v1_iot(request):
    if request.method == 'POST':
        serializer = iotv1Serializer(data=request.data)
        if serializer.is_valid():
            try:
                bot = ioTank.objects.get(bot_user=request.user)
                rtd = serializer.validated_data.get('rtd')
                ec = serializer.validated_data.get('ec')
                do = serializer.validated_data.get('do')
                ph = serializer.validated_data.get('ph')
                orp = serializer.validated_data.get('orp')
                ip = serializer.validated_data.get('ip')
                rssi = serializer.validated_data.get('rssi')

                bot.rssi = rssi
                bot.ip = ip
                bot.save()

                owner = bot.owner
                settings = AccountSettings.objects.get(user=owner)
                #serializer.save(bot=ioTank.objects.get(bot_user=request.user), rtd=rtd, ec=ec, do=do, ph=ph, orp=orp)
                SensorReading.objects.create(bot=ioTank.objects.get(bot_user=request.user), rtd=rtd, ec=ec, do=do, ph=ph, orp=orp)

                activate(settings.timezone)
                sub = "🦑Hydra Notice"
                red_flags = []

                # now
                now = timezone.localtime(timezone.now()).time()
#
# AUTOMATION CODE WOULD GO HERE 
#
                return Response(timezone.now(), status=status.HTTP_201_CREATED)
            except ObjectDoesNotExist:
                return Response(serializer.errors, status=status.HTTP_404_NOT_FOUND)

        return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)
