import uuid

from django.contrib.auth.models import User
from django.db import models
from django.utils.translation import ugettext_lazy as _
from timezone_field import TimeZoneField

NOTICE_TYPE = (
    ('E', 'E-mail'),
)

class Notice(models.Model):
    owner = models.ForeignKey(User, on_delete=models.CASCADE)
    taskid = models.CharField(_('Task ID'), max_length=36)
    type = models.CharField(max_length=1, choices=NOTICE_TYPE)
    timestamp = models.DateTimeField(auto_now_add=True, editable=False)

    class Meta:
        verbose_name = _('Notice')
        verbose_name_plural = _('Notices')
        ordering = ('owner',)

    def __str__(self):
        return str(self.taskid)

    def __unicode__(self):
        return str(self.taskid)

NOTICE_TYPE = (
    ('I', 'Hydra Emergency'),
)

class Notices(models.Model):
    sentdt = models.DateTimeField(auto_now_add=True, editable=False)
    user = models.ForeignKey(User, on_delete=models.CASCADE)
    type = models.CharField(max_length=1, choices=NOTICE_TYPE)
    value = models.CharField(max_length=50)

class AccountSettings(models.Model):
    user = models.ForeignKey(User, on_delete=models.CASCADE)
    timezone = TimeZoneField()
    notify_iotank_emergency = models.BooleanField(default=True)
    notify_email = models.BooleanField(default=True)
    email_daily = models.PositiveSmallIntegerField(default=1)

    def __str__(self):
        return str(self.user)

    def __unicode__(self):
        return str(self.user)

    class Meta:
        verbose_name = 'Account Setting'
        verbose_name_plural = 'Account Settings'

from django.utils import timezone

UNIT = (
    ('C', 'Celsius'),
    ('F', 'Fahrenheit'),
)

# Create your models here.
class ioTank(models.Model):
    bot_user = models.ForeignKey(User, related_name="bot_owner", on_delete=models.CASCADE)
    owner = models.ForeignKey(User, on_delete=models.CASCADE)
    id = models.UUIDField(primary_key=True, default=uuid.uuid4, editable=False)
    created_at = models.DateTimeField(auto_now_add=True)
    u = models.CharField(_('Temperature Unit'), max_length=1, default='F', choices=UNIT)
    name = models.CharField(max_length=100, blank=True, null=True, )
    rssi = models.DecimalField(_('RSSI'), max_digits=5, decimal_places=2, default=0)
    ip = models.CharField(max_length=45, blank=True, null=True, )


    def __str__(self):
        return str(self.id)

    class Meta:
        verbose_name = 'Hydra'
        verbose_name_plural = 'Hydra'

class SensorReading(models.Model):
    bot = models.ForeignKey(ioTank, editable=False, on_delete=models.CASCADE)
    rtd = models.DecimalField(_('Waterproof Temp'), max_digits=7, decimal_places=3, default=0)
    ec = models.DecimalField(_('Conductivity'), max_digits=7, decimal_places=3, default=0)
    do = models.DecimalField(_('Dissolved Oxygen'), max_digits=7, decimal_places=3, default=0)
    ph = models.DecimalField(_('pH'), max_digits=7, decimal_places=3, default=0)
    orp = models.DecimalField(_('ORP'), max_digits=7, decimal_places=3, default=0)
    timestamp = models.DateTimeField(editable=False)

    def save(self, *args, **kwargs):
        if not self.timestamp:
            self.timestamp = (timezone.now())
        super(SensorReading, self).save(*args, **kwargs)

    def __str__(self):
        return str(self.timestamp)

    class Meta:
        verbose_name = 'Sensor Reading'
        verbose_name_plural = 'Sensor Readings'
